/* File:                DH_keyexchange
 * Description:         Contains the Diffie Hellman Keyexchange 
 *                      implementation for the SSH protocl.
 * Author:              Magnus Leksell
 * 
 * Copyright © 2006-2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/

#include "DH_keyexchange.h"
#include <iostream>
#include "ssh_transport.h"
#include "definitions.h"
#include "sshmessages.h"
#include <openssl/evp.h>
#include <assert.h>
#include "HashStream.h"
#include "sha1.h"
#include "common.h"

void dump(char * name, unsigned char * data, uint32 len);

namespace ssh
{

    using namespace std;

    /* Function:        DH_keyexchange::generate_keys
     * Description:     Generates the clients public and private key.
     */
    bool DH_keyexchange::generate_keys()
    {
        m_dh = DH_new();
        if(!m_dh) {
            return false;
        }
        // first convert the large prime
        if(!BN_hex2bn(&m_dh->p,m_p)) {
            DH_free(m_dh); m_dh = 0;
            return false;
        }
        // now convert the generator
        if(!BN_hex2bn(&m_dh->g,this->m_g)) {
            DH_free(m_dh); m_dh = 0;
            return false;
        }
        // now generate the key.
        if(!DH_generate_key(m_dh)) {
            DH_free(m_dh); m_dh = 0;
            return false;
        }
        // return success.
        return true;
    }

    /* Function:        DH_keyexchange::perform_keyexchange
     * Description:     Performs the actual keyexchange.
     */
    int DH_keyexchange::perform_keyexchange(ssh_transport * ssh_tran, exchange_data * data)
    {
        if(!generate_keys()) {
            DebugPrint("Failed to generate keys")
            return KEY_GENERATION_FAILED;
        }
        ssh_tran->newPacket();
        /* create the SSH_MSG_KEXDH_INIT message */
        if(!ssh_tran->writeByte(SSH_MSG_KEXDH_INIT) ||
            !ssh_tran->writeBigInteger(m_dh->pub_key)) 
        {
            DebugPrint("Failed to write KEXDH init packet to stream")
            return KEYEXCHANGE_FAILED;
        }   
        if(ssh_tran->exchangeAndExpect(SSH_MSG_KEXDH_REPLY) != STATUS_SUCCESS)
        {
            DebugPrint("Failed to exchange Diffie-Hellman keyexchange init packets")
            return KEYEXCHANGE_FAILED;
        }
        
        if((parse_kexdh_reply(ssh_tran, data) != STATUS_SUCCESS) || 
            (calculate_shared_secret(data) != STATUS_SUCCESS))
        {
            /* perform the required cleanup */
            SAFE_DELETE_ARRAY(data->keyblob)
            SAFE_DELETE_ARRAY(data->signature)
            if(data->server_pubkey) {
                BN_free(data->server_pubkey);
                data->server_pubkey = 0;
            }
            return KEYEXCHANGE_FAILED;
        }

        return STATUS_SUCCESS;
    }


    /* Function:        parse_kexdh_reply
     * Description:     Parses the kexdh reply sent from the server and 
     *                  extracts all the required data to be able to continue
     *                  with the keyexchange.
     */
    int DH_keyexchange::parse_kexdh_reply(ssh_transport * ssh_tran, exchange_data * data)
    {
        unsigned char type;
        if(!ssh_tran->readByte(type)) {
            return KEXDH_PARSE_ERROR;
        }
        if(!ssh_tran->readInt32(data->keyblob_len))
        {
            DebugPrint("failed to extract keyblob from packet")
            return KEXDH_PARSE_ERROR;
        }
        /* Check if the size is valid */
        if(data->keyblob_len > MAX_KEYBLOB_LENGTH)
        {
            DebugPrint("The keyblob is to large")
            return KEYEXCHANGE_FAILED;
        }
        data->keyblob = new (std::nothrow) unsigned char[data->keyblob_len];
        if(data->keyblob == NULL) {
            DebugPrint("Failed to allocate memory for the keyblob")
            return MEMORY_ALLOCATION_FAILED;
        }
        /* read the keyblob */
        if(!ssh_tran->readBytes(data->keyblob, data->keyblob_len))
        {
            DebugPrint("Failed to read the keyblob")
            return KEXDH_PARSE_ERROR;
        }
        /* Read the servers public key */
        if(!ssh_tran->readBigInteger(&data->server_pubkey))
        {
            DebugPrint("Failed to read the hosts public key")
            return KEXDH_PARSE_ERROR;
        }
        /* Read the length of the signature */
        if(!ssh_tran->readInt32(data->signature_len))
        {
            DebugPrint("Parse error")
            return KEXDH_PARSE_ERROR;
        }
        /* check the length */
        if(data->signature_len > MAX_SIGNATURE_LENGTH) {
            DebugPrint("Signature size to big")
            return KEYEXCHANGE_FAILED;
        }
        // allocate memory for the signature
        data->signature = new (std::nothrow) unsigned char[data->signature_len];
        if(data->signature == NULL) {
            DebugPrint("Memory allocation failed")
            return MEMORY_ALLOCATION_FAILED;
        }
        /* read the signature */
        if(!ssh_tran->readBytes(data->signature, data->signature_len)) {
            DebugPrint("Failed to read signature")
            return KEXDH_PARSE_ERROR;
        }
        return STATUS_SUCCESS;
    }

    /* Function:        DH_keyexchange::calculate_shared_secret
     * Description:     Calculates the shared secret and stores it in the
     *                  supplied struct.
     */
    int DH_keyexchange::calculate_shared_secret(exchange_data * data)
    {
        unsigned char buffer[MAX_SECRET_LENGTH];
        int length = DH_size(m_dh);
        if(length <= 0) 
            return KEYEXCHANGE_FAILED;

        int slen = DH_compute_key(buffer, data->server_pubkey, m_dh);
        if(slen <= 0) {
            DebugPrint("Failed to calculate the shared secret")
            return KEYEXCHANGE_FAILED;
        }
        //dump("shared secret", buffer, slen);

        data->shared_secret = BN_bin2bn(buffer, slen, 0);
        if(data->shared_secret == NULL) {
            return KEYEXCHANGE_FAILED;
        }
        return STATUS_SUCCESS;
    }

    /* Function:        DH_keyexchange::calculate_exchangehash
     * Description:     Calculates the exchange hash. Required for the key derivation process.
     */
    int DH_keyexchange::calculate_exchangehash(const ssh_context & local_context, 
                                               const ssh_context & server_context, 
                                               exchange_data * exdata)
    {
        HashStream hashStream("sha1");
        if(!hashStream) 
            return FATAL_ERROR;

        /* write the required data to the buffer */
        hashStream.writeString(local_context.protocolVersion);          
        hashStream.writeString(server_context.protocolVersion);

        /* write the local kex message */
        hashStream.writeInt32(exdata->client_kex_hdr.packet_size - exdata->client_kex_hdr.padding_size - 1);
        hashStream.writeByte(SSH_MSG_KEXINIT);
        hashStream.writeBytes(local_context.cookie, 16);                    // the random bytes, the "cookie"
        hashStream.writeString(local_context.kexs);                         // the key exchange algorithms
        hashStream.writeString(local_context.hostkeys);                     // the hostkeys
        hashStream.writeString(local_context.ciphers_client_to_server);     // the ciphers 
        hashStream.writeString(local_context.ciphers_server_to_client);     // the ciphers, again
        hashStream.writeString(local_context.hmacs_client_to_server);       // the hmacs
        hashStream.writeString(local_context.hmacs_server_to_client);       // the hmacs
        hashStream.writeString(local_context.compression_client_to_server); // compression algorithms
        hashStream.writeString(local_context.compression_server_to_client); // compression algorithms
        hashStream.writeInt32(0);                                           // languages
        hashStream.writeInt32(0);                                           // languages
        hashStream.writeByte(0);                                
        hashStream.writeInt32(0);
        
        /* write the servers kex message */
        hashStream.writeInt32(exdata->server_kex_hdr.packet_size - exdata->server_kex_hdr.padding_size - 1);
        hashStream.writeByte(SSH_MSG_KEXINIT);
        hashStream.writeBytes(server_context.cookie, 16);                       // the random bytes, the "cookie"
        hashStream.writeString(server_context.kexs);                            // the key exchange algorithms
        hashStream.writeString(server_context.hostkeys);                        // the hostkeys
        hashStream.writeString(server_context.ciphers_client_to_server);        // the ciphers 
        hashStream.writeString(server_context.ciphers_server_to_client);        // the ciphers, again
        hashStream.writeString(server_context.hmacs_client_to_server);          // the hmacs
        hashStream.writeString(server_context.hmacs_server_to_client);          // the hmacs
        hashStream.writeString(server_context.compression_client_to_server);    // compression algorithms
        hashStream.writeString(server_context.compression_server_to_client);    // compression algorithms
        hashStream.writeString(server_context.languages_client_to_server);      // languages
        hashStream.writeString(server_context.languages_server_to_client);      // languages
        hashStream.writeByte(server_context.kex_follows);               
        hashStream.writeInt32(0);
        
        hashStream.writeString(exdata->keyblob, exdata->keyblob_len);           // the signature
        hashStream.writeBigInteger(m_dh->pub_key);                              // the clients public key
        hashStream.writeBigInteger(exdata->server_pubkey);                      // the servers public key       
        hashStream.writeBigInteger(exdata->shared_secret);                      // the shared secret

        uint32 num;
        unsigned char digest[MAX_DIGEST_SIZE];
        // now finalize the hash.
        hashStream.digest(digest, &num);

        // allocate and copy the data.
        exdata->exchange_hash = new (std::nothrow) unsigned char[num];
        if(exdata->exchange_hash == NULL) {
            DebugPrint("Memory allocation failed")
            return FATAL_ERROR;
        }
        memcpy(exdata->exchange_hash, digest, num);
        exdata->exchange_len = num;
        
        // success
        return STATUS_SUCCESS;
    }
};