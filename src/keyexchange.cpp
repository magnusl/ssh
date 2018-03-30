/* File:            keyexchange.cpp
 * Description:     Inplements all the functions required for the SSH
 *                  keyexchange.
 * Author:          Magnus Leksell
 * Copyright 2006-2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/

#include "ssh_transport.h"
#include <iostream>
#include "kex_base.h"
#include "sshmessages.h"
#include <assert.h>
#include "definitions.h"
#include "StringUtil.h"
#include "ArrayStream.h"
#include "sha1.h"
#include "md5.h"
#include "HashStream.h"
#include "common.h"

void dump(char * name, unsigned char * data, uint32 len);

namespace ssh
{

    using namespace std;

    /* Function:                exchange_kex
     * Description:             Exchanges the kex messages required.
     *
     * Bugfixes and updates:
     *****************************************************************************/

    bool ssh_transport::exchange_kex(bool bOnlyClientKex = false)
    {
        int nret;
        newPacket();    
        /* 
         * now write the local kex 
         */
        if(!writeByte(SSH_MSG_KEXINIT) ||  
            !writeBytes(local_context.cookie, 16) ||                        // the random bytes, the "cookie"
            !writeString(local_context.kexs) ||                             // the key exchange algorithms
            !writeString(local_context.hostkeys) ||                         // the hostkeys
            !writeString(local_context.ciphers_client_to_server) ||         // the ciphers 
            !writeString(local_context.ciphers_server_to_client) ||         // the ciphers, again
            !writeString(local_context.hmacs_client_to_server) ||           // the hmacs
            !writeString(local_context.hmacs_server_to_client) ||           // the hmacs
            !writeString(local_context.compression_client_to_server) ||     // compression algorithms
            !writeString(local_context.compression_server_to_client) ||     // compression algorithms
            !writeInt32(0) ||                                               // languages
            !writeInt32(0) ||                                               // languages
            !writeByte(0) ||                                
            !writeInt32(0))
        {
            disconnect(L"Internal Error during keyexchange", SSH_DISCONNECT_KEY_EXCHANGE_FAILED);
            DebugPrint("Failed to exchange the keyexchange init message");
            return false;
        }
        if(bOnlyClientKex) {
            // Only send the client keyexchange packet
            nret = sendpacket();
        } else {
            // send the client keyexchange packet and expect the servers
            nret = exchangeAndExpect(SSH_MSG_KEXINIT);
        }
        if(nret < 0) {
            return false;
        } else if(nret == WRONG_PACKET) {
            disconnect(L"Expected a SSH_MSG_KEXINIT packet, but got another", SSH_DISCONNECT_PROTOCOL_ERROR);
            DebugPrint("Unexpected message received during the keyexchange phase");
            SetLastError(SSH_DISCONNECT_PROTOCOL_ERROR);
            return false;
        }
        return true;
    }

    /* Function:        ssh_transport::perform_initial_keyexchange
     * Description:     performs the initial keyexchange.
     */
    int ssh_transport::perform_keyexchange(bool bPacketsExchanged = false)
    {
        exchange_data data;
        if(bPacketsExchanged == false) {
            /* exchange the keyexchange init messages */
            if(!exchange_kex()) {
                DebugPrint("Failed to exchange the keyexchange initalization messages");
                SetLastError(SSH_DISCONNECT_KEY_EXCHANGE_FAILED);
                return FATAL_ERROR;
            }
        }
        /* Save the headers */
        data.server_kex_hdr = m_lastReadHdr;
        data.client_kex_hdr = m_lastSentHdr;

        /* now parse the kex message sent by the servr */
        int nret = parse_server_kex();
        if(nret != STATUS_SUCCESS)
            return FATAL_ERROR;
        
        /* Decide what we should use */
        algorithm_list algorithms;
        if(!decide_algorithms(algorithms)) {
            return FATAL_ERROR;
        }

        kex_base *      kexInstance         = 0;        // used for key-derivation
        hostkey_base *  hostkeyInstance     = 0;        // used for host-authentication.

        // containers for the selected algorithms.
        Algorithms server_algorithms, client_algorithms;
        /* initalize the algorithm instances */
        nret = initalize_algorithms(algorithms, client_algorithms, server_algorithms, &kexInstance, &hostkeyInstance);
        if(nret < 0) return FATAL_ERROR;

        /* now perform the actual keyexchange using the selected algorithm */
        if(kexInstance->perform_keyexchange(this, &data) != STATUS_SUCCESS)                             
            return FATAL_ERROR;

        if(kexInstance->calculate_exchangehash(local_context,server_context, &data) != STATUS_SUCCESS) 
            return FATAL_ERROR;

        if(hostkeyInstance) /* Host verification */
        {
            if(!hostkeyInstance->parse_keyblob(data.keyblob, data.keyblob_len) ||                               // parse the keyblob
                !hostkeyInstance->parse_signature(data.signature, data.signature_len) ||                        // parse the signature
                (hostkeyInstance->verify_host(data.exchange_hash, data.exchange_len) != VALID_SIGNATURE) ||     // verify the host
                !check_fingerprint(hostkeyInstance))                                                            // check the fingerprint.
            {
                if(kexInstance)     delete kexInstance;
                if(hostkeyInstance) delete hostkeyInstance;
                return FATAL_ERROR;
            }
        }
        if(sessionIdentifier.empty()) {
            /* Use this as the session identifier */
            sessionIdentifier.resize(data.exchange_len);
            memcpy(&sessionIdentifier[0],data.exchange_hash, data.exchange_len);
        }

        // Now derive the keys.
        if(derive_keys(kexInstance, client_algorithms , server_algorithms, &data) != STATUS_SUCCESS) {
            if(kexInstance)     delete kexInstance;
            if(hostkeyInstance) delete hostkeyInstance;
            return FATAL_ERROR;
        }

        // don't need these anymore.
        if(kexInstance)     delete kexInstance;
        if(hostkeyInstance) delete hostkeyInstance;

        /* now send a newkey message before we take the new keys into action */
        newPacket();
        if(!writeByte(SSH_MSG_NEWKEYS)) return FATAL_ERROR;
        if(exchangeAndExpect(SSH_MSG_NEWKEYS) != STATUS_SUCCESS)
            return FATAL_ERROR;
        setAlgorithms(client_algorithms, server_algorithms);
        /* Update the keyexchange time */
        m_lastKeyExchange = time(NULL);
        /* keyexchange complete, return successfully */
        return STATUS_SUCCESS;
    }

    /* Function:        ssh_transport::check_fingerprint
     * Description:     Calculates the fingerprint of the servers public key and compares it 
     *                  with the one stored in the cache.
     */
    bool ssh_transport::check_fingerprint(const hostkey_base * hkey)
    {
        string fp, saved_fp;
        /* Create a fingerprint of the servers public key */
        hkey->fingerprint("md5",fp);
        /* Check if the host is in the cache of known hosts */
        if(m_knownHosts.getFingerprint(m_host, saved_fp)) {
            // do they match?
            if(fp != saved_fp) {
                if(m_notify) {
                    int action = m_notify->OnChangedFingerprint(m_host, fp);
                    switch(action) 
                    {
                    case HOST_ACCEPT_TEMP:
                        break;
                    case HOST_REPLACE:
                        m_knownHosts.replaceFingerprint(m_host, fp);
                        break;
                    case HOST_ERROR:
                        cerr << "ssh_transport::perform_keyexchange: The client didn't accept the host" << endl;
                        return false;
                    default:
                        cerr << "ssh_transport::perform_keyexchange: Callback function returned a invalid value (OnChangedFingerprint)" << endl;
                        return false;
                    }
                } else {
                    return false;
                } 
            } 
        } else {
            if(m_notify) {
                int action = m_notify->OnUnknownHost(m_host, fp);
                switch(action)
                {
                case HOST_ERROR:
                    cerr << "ssh_transport::perform_keyexchange: The client didn't accept the host" << endl;
                    return false;
                case HOST_ACCEPT_TEMP:
                    break;
                case HOST_ACCEPT_PERM:
                    // add the fingerprint to the cache
                    m_knownHosts.addFingerprint(m_host, fp);
                    // flush it to disk
                    m_knownHosts.flush();
                    break;
                default:
                    cerr << "ssh_transport::perform_keyexchange: Callback function returned a invalid value OnUnknownHost" << endl;
                    return false;
                }
            }
            else
                return false;
        }
        return true;
    }

    /* Function:        ssh_transport::parse_server_kex
     * Description:     Parses the KEXINIT message sent by the server, the sent 
     *                  message MUST be stored in the readState.msg variable.
     *
     * Bugfixes and updates:    none
     *****************************************************************************/
    int ssh_transport::parse_server_kex()
    {
        // make sure that we are working on the correct message
        if(readState.msg.hdr->s.type != SSH_MSG_KEXINIT)
            return WRONG_PACKET;

        /* just skip the type now  */
        unsigned char type;
        if(!readByte(type)) {
            cerr << "Error while parsing SSH_MSG_KEXINIT packet, error while reading identifier" << endl;
            disconnect(L"Could not read the expected data from the packet", SSH_DISCONNECT_PROTOCOL_ERROR);
            return KEX_PARSE_ERROR;
        }

        // now read the data.
        if(!readBytes(server_context.cookie, 16) || 
            !readString(server_context.kexs) ||
            !readString(server_context.hostkeys) ||
            !readString(server_context.ciphers_client_to_server) ||
            !readString(server_context.ciphers_server_to_client) ||
            !readString(server_context.hmacs_client_to_server) ||
            !readString(server_context.hmacs_server_to_client) ||
            !readString(server_context.compression_client_to_server) ||
            !readString(server_context.compression_server_to_client) ||
            !readString(server_context.languages_client_to_server) ||
            !readString(server_context.languages_server_to_client) ||
            !readByte(server_context.kex_follows) ||
            !readInt32(server_context.reserved))
        {
            cerr << "Error while parsing server kex" << endl;
            disconnect(L"Could not read the expected data from the packet", SSH_DISCONNECT_PROTOCOL_ERROR);
            return KEX_PARSE_ERROR;
        }
        return STATUS_SUCCESS;
    }

    /* Function:            ssh_transport::decide_algorithms
     * Description:         Decides what algorithms to use. Compares the local namelists with
     *                      the namelists sent by the server.
     */
    bool ssh_transport::decide_algorithms(algorithm_list & list)
    {
        /* the keyexchange algorithm */
        if(!decide(local_context.kexs, server_context.kexs, list.keyexchange_algorithm)) {
            DebugPrint("Failed to find a matching keyexchange algorithm");
            return false;
        }
        /* the hostkey algorithm */
        if(!decide(local_context.hostkeys, server_context.hostkeys, list.hostkey_algorithm)) {
            DebugPrint("Failed to find a matching hostkey algorithm");
            return false;
        }
        /* cipher, client to server direction */
        if(!decide(local_context.ciphers_client_to_server, server_context.ciphers_client_to_server, 
            list.cipher_server_algorithm)) 
        {
            DebugPrint("Failed to find a matching cipher algorithm");
            return false;
        }
        /* cipher, server to client direction */
        if(!decide(local_context.ciphers_server_to_client,server_context.ciphers_server_to_client, 
            list.cipher_client_algorithm))
        {
            DebugPrint("Failed to find a matching cipher algorithm");
            return false;
        }
        /* hmac, server to client direction */
        if(!decide(local_context.hmacs_server_to_client, server_context.hmacs_server_to_client,  
            list.hmac_server_algorithm))
        {
            DebugPrint("Failed to find matching hmac");
            return false;
        }
        /* hmac, client to server direction */
        if(!decide(local_context.hmacs_client_to_server, server_context.hmacs_client_to_server,
            list.hmac_client_algorithm)) 
        {
            DebugPrint("Failed to find matching hmac");
            return false;
        }
        /* compression, server to client direction */
        if(!decide( local_context.compression_client_to_server, server_context.compression_server_to_client,
            list.compression_client_algorithm))
        {
            DebugPrint("Failed to find matching compression algorithm");
            return false;
        }
        /* compression, client to server direction */
        if(!decide(local_context.compression_client_to_server, server_context.compression_client_to_server, 
            list.compression_server_algorithm))
        {
            DebugPrint("Failed to find matching compression algorithm");
            return false;
        }
        // return success.
        return true;
    }

    /* Function:        ssh_transport::derive_keys
     * Description:     Derives the required keys for the encryption/decryption ciphers and
     *                  the keyed message authentication code. All the algorithms are initalized and
     *                  and ready to use if this function succedes.
     * 
     * Bugfixes and updates:
     */
    int ssh_transport::derive_keys(kex_base * kex, Algorithms & clientdir, Algorithms & serverdir,const exchange_data * exchangeData)
    {
        unsigned char key[MAX_KEY_LENGTH], integrity[MAX_KEY_LENGTH], iv[MAX_IV_LENGTH];
        
        /* first derive the keys for the client algorithms */
        uint32 key_length = clientdir.cipher->getKeySize(), iv_length = clientdir.cipher->getBlockSize(),
            integrity_length = clientdir.hmac->getKeyLength();

        if(derive_key(exchangeData, 'D', key, key_length, kex) != STATUS_SUCCESS) {
            cerr << "Failed to derive the key for the client cipher algorithm" << endl;
            return STATUS_FAILURE;
        }

        if(derive_key(exchangeData, 'B', iv, iv_length, kex) != STATUS_SUCCESS) {
            cerr << "Failed to derive IV for the client cipher algorithm" << endl;
            return STATUS_FAILURE;
        }

        if(derive_key(exchangeData, 'F', integrity, integrity_length, kex) != STATUS_SUCCESS) {
            cerr << "Failed to derive key for the client cipher algorithm" << endl;
            return STATUS_FAILURE;
        }

        if(!clientdir.cipher->decryptInit(key, iv)) {
            cerr << "Failed to initalize the decryption cipher" << endl;
            return STATUS_FAILURE;
        }

        if(!clientdir.hmac->init(integrity)) {
            cerr << "Failed to initalize the hmac for the client side" << endl;
            return STATUS_FAILURE;
        }

        /* initalize the server direction algorithms */
        key_length = serverdir.cipher->getKeySize(), iv_length = serverdir.cipher->getBlockSize(),
            integrity_length = serverdir.hmac->getKeyLength();

        if(derive_key(exchangeData, 'C', key, key_length, kex) != STATUS_SUCCESS) {
            cerr << "Failed to derive the key for the client server algorithm" << endl;
            return STATUS_FAILURE;
        }
        if(derive_key(exchangeData, 'A', iv, iv_length, kex) != STATUS_SUCCESS) {
            cerr << "Failed to derive IV for the server cipher algorithm" << endl;
            return STATUS_FAILURE;
        }

        if(derive_key(exchangeData, 'E', integrity, integrity_length, kex) != STATUS_SUCCESS) {
            cerr << "Failed to derive key for the server cipher algorithm" << endl;
            return STATUS_FAILURE;
        }

        if(!serverdir.cipher->encryptInit(key, iv)) {
            cerr << "Failed to initalize the encryption cipher" << endl;
            return STATUS_FAILURE;
        }

        if(!serverdir.hmac->init(integrity)) {
            cerr << "Failed to initalize the hmac for the server side" << endl;
            return STATUS_FAILURE;
        }

        return STATUS_SUCCESS;
    }

    /* Function:        ssh_transport::derive_key
     * Description:     Derives a key for the cipher or the hmac
     */
    int ssh_transport::derive_key(const exchange_data * exdata, char unique, unsigned char * key, uint32 len, 
                                  kex_base * kex)
    {
        unsigned char digest[MAX_DERIVATION_SIZE + MAX_DIGEST_LENGTH];  // make sure that we don't overflow the buffer.
        if(len > MAX_DERIVATION_SIZE)
            return STATUS_FAILURE;

        ssh::HashStream hash(kex->hash());

        uint32 current_length = 0, dlen = kex->get_digest_length();
        // first write the shared secret.
        if(!hash.writeBigInteger(exdata->shared_secret) ||
            !hash.writeBytes(exdata->exchange_hash, exdata->exchange_len) ||
            !hash.writeByte((unsigned char)unique) ||
            !hash.writeBytes(&sessionIdentifier[0], (uint32) sessionIdentifier.size()))
        {
            return STATUS_FAILURE;
        }
        // calculate the digest,
        hash.digest(digest, &dlen);
        current_length += dlen;

        /* loop until we have the required data */
        while(current_length < len)
        {
            hash.clear();
            if(!hash.writeBigInteger(exdata->shared_secret) ||
                !hash.writeBytes(exdata->exchange_hash, exdata->exchange_len) ||
                !hash.writeBytes(digest, current_length))
            {
                return STATUS_FAILURE;
            }
            hash.digest(digest + current_length, &dlen);
            if(dlen <= 0)
                return STATUS_FAILURE;
            current_length += dlen;
        }
        memcpy(key, digest, len);       // copy the required amount of data to the key buffer.
        return STATUS_SUCCESS;
    }

    /* Function:        ssh_transport::timeSinceKeyExchange
     * Description:     Returns the number of seconds since the last keyexchange.
     */
    uint32 ssh_transport::timeSinceKeyExchange()
    {
        time_t currentTime = time(NULL);
        return static_cast<uint32>(difftime(currentTime, m_lastKeyExchange));
    }
};