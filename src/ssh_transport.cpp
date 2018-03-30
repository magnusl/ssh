/* File:            ssh_transport
 * Description:     Implements the SSH2 Transport Protocol. The transport 
 *                  protocol provides a secure reliable bytestream for services. 
 * Author:          Magnus Leksell
 *
 * Copyright © 2006-2007 Magnus Leksell, all rights reserved
 *****************************************************************************/

#include <iostream>
#include <vector>
#include <assert.h>
#include <ctime>

#include "ssh_transport.h"
#include "misc.h"
#include "protocol_version.h"
#include "definitions.h"
#include "types.h"
#include "factories.h"
#include "sshmessages.h"
#include "ArrayStream.h"
#include "StringUtil.h"
#include "common.h"

namespace ssh
{

    using namespace std;

    /* Function:        ssh_transport::ssh_transport
     * Description:     The constructor, initalizes the transport layer.
     */
    ssh_transport::ssh_transport(ssh_notify * notify)
    {
        m_notify                = notify;
        sendState.msg.payload   = NULL;
        readState.msg.payload   = NULL;
        sendState.msg.data      = NULL;
        readState.msg.data      = NULL;
        sendState.state         = STATE_NO_PACKET;
        readState.state         = STATE_NO_PACKET;

    }

    /* Function:        ssh_transport::~ssh_transport()
     * Description:     Performs the required cleanup.
     */
    ssh_transport::~ssh_transport()
    {
        algorithm_cleanup();                        // the algorithms
        SAFE_DELETE_ARRAY(sendState.msg.data)       // delete the send buffer
        SAFE_DELETE_ARRAY(readState.msg.data)       // delete the read buffer
    }

    /* Function:        ssh_transport::force_disconnect
     * Description:     Forces a disconnect, disconnects without sending a disconnect message.
     */
    void ssh_transport::force_disconnect()
    {
        socketLayer.disconnect();
    }

    /* Function:        ssh_transport::disconnect
     * Description:     Sends a SSH_MSG_DISCONNECT messages and then disconnects.
     */
    void ssh_transport::disconnect(const wchar_t * error_string, uint32 reason)
    {
        int nret;
        if(sending_packet()) {
            nret = sendpacket(20);
            if(nret == DISCONNECTED) {
                DebugPrint("Disconnected by the server")
            } else if(nret != STATUS_SUCCESS) {
                DebugPrint("Could not finish the last packet before disconnecting")
                socketLayer.disconnect();
            }
        } else {
            // Write the disconnect message to the buffer
            newPacket();
            if(!writeByte(SSH_MSG_DISCONNECT) ||    // message type
                !writeInt32(reason) ||              // reason code
                !writeWideString(error_string) ||   // error string
                !writeInt32(0))                     // language tag.
            {
                DebugPrint("Error while writing the disconnect message")
                socketLayer.disconnect();
            } else {
                // send the packet
                nret = sendpacket(20);
                if(nret == DISCONNECTED) {
                    DebugPrint("Disconnected by the server")
                } else {
                    socketLayer.disconnect();
                }
            }
        }
    }

    /* Function:        ssh_transport::initalize
     * Description:     Initalizes the transport layer. The transport layer has two
     *                  internal buffers, one for sending and one for reading data. 
     */
    bool ssh_transport::initalize(ConfigReader & config)
    {
        std::string enc,                    // Encryption algorithms
                    hmac,                   // Integrity algorithms
                    kex,                    // Keyexchange algorithms
                    hkey,                   // The hostkeys
                    ApplicationIdentifier,  // Identifies the application.
                    hosts;  

        /* Allocate the required memory for the buffers */

        // Send buffer.
        sendState.msg.data = new (std::nothrow) unsigned char [MAX_SSH_PACKETSIZE + MAX_DIGEST_LENGTH];
        if(sendState.msg.data == NULL) {
            setLastError(MEMORY_ALLOCATION_FAILED, "Failed to allocate memory for the send buffer");
            DebugPrint("Failed to allocate memory for the send buffer")
            return false;
        }
        // Read buffer
        readState.msg.data = new (std::nothrow) unsigned char [MAX_SSH_PACKETSIZE + MAX_DIGEST_LENGTH];
        if(readState.msg.data == NULL) {
            setLastError(MEMORY_ALLOCATION_FAILED, "Failed to allocate memory for the read buffer");
            DebugPrint("Failed to allocate memory for the read buffer")
            SAFE_DELETE(sendState.msg.data)
            return false;
        }

        // set the pointers
        sendState.msg.hdr = (ssh_msg::packet_hdr *)sendState.msg.data;
        sendState.msg.payload = sendState.msg.data + sizeof(ssh_hdr);
        sendState.msg.payload_usage = 0;
        sendState.total_bytes_transfered = 0;
        readState.msg.hdr = (ssh_msg::packet_hdr *)readState.msg.data;
        readState.msg.payload = readState.msg.data + sizeof(ssh_hdr);
        readState.msg.payload_usage = 0;
        readState.total_bytes_transfered = 0;

        // initalize the states
        readState.state = STATE_NO_PACKET;
        readState.state = STATE_NO_PACKET;

        // Encryption algorithms
        if(!config.readString("ssh.security.prefered_ciphers", enc))        
            enc = cipher_base::DEFAULT_CIPHERS;
        // The integrity algorithms
        if(!config.readString("ssh.security.prefered_hmacs", hmac))     
            hmac = hmac_base::DEFAULT_HMACS;
        // The keyexchange algorithms
        if(!config.readString("ssh.security.keyexchange",kex))  
            kex = kex_base::DEFAULT_KEXS;
        // Host verification algorithms
        if(!config.readString("ssh.security.hostkey", hkey))        
            hkey = "ssh-rsa,ssh-dss";   
        // load the known hosts.
        if(!config.readString("ssh.security.hostcache", hosts)) 
            hosts = "hosts.txt";
        // load the file with the known hosts.
        if(!m_knownHosts.load(hosts.c_str())) return false;

        /* initalize the local context */
        local_context.ciphers_client_to_server = local_context.ciphers_server_to_client = enc;      // encryption
        local_context.hmacs_client_to_server = local_context.hmacs_server_to_client     = hmac;     // integrity
        local_context.kexs                                                              = kex;      // Keyexchange
        local_context.hostkeys                                                          = hkey;     // Host verification.           
        local_context.compression_server_to_client = "none";
        local_context.compression_client_to_server = "none";                            // Compression not supported
        local_context.protocolVersion = "SSH-2.0-Magnus.SSHLib";                        // Use this for now.
        return true;
    }

    /* Function:        ssh_transport::connect
     * Description:     Connects to the server and starts the connection.
     */
    int ssh_transport::connect(const char * addr, const char * port, Event * abortEvent)
    {
        m_host = addr;
        cerr << "Socket layer connecting" << endl;
        int nret = socketLayer.connect(addr, port,30, abortEvent);
        cerr << "after socketLayer.connect" << endl;
        if(nret != STATUS_SUCCESS)
        {
            switch(nret)
            {
            case HOST_LOOKUP_FAILED:
                setLastError(HOST_LOOKUP_FAILED, "Failed to find the specified host");
                return HOST_LOOKUP_FAILED;
            case NETWORK_ERROR:
                setLastError(NETWORK_ERROR, "A fatal internal network error occurred");
                return NETWORK_ERROR;
            case CONNECTION_FAILED:
                setLastError(CONNECTION_FAILED, "Failed to connect to the server");
                return CONNECTION_FAILED;
            default:
                setLastError(UNKNOWN_ERROR, "A unknown error occurred while trying to connect to the server");
                return UNKNOWN_ERROR;
            }
        }

        /* Connected to the server , now exchange the protocol versions */
        if((nret = exchange_protocol_versions()) != STATUS_SUCCESS)
        {
            switch(nret)
            {
            case CONNECTION_FAILED:
                cerr << "Failed to exchange the protocol version strings" << endl;
                return CONNECTION_FAILED;
            case UNSUPPORTED_PROTOCOL_VERSION:
                cerr << "The server protocol version is not supported, only SSH2 is supported" << endl;
                return UNSUPPORTED_PROTOCOL_VERSION;
            default:
                cerr << "A unknown error occurred during the protocol version exchange phase" << endl;
                return UNKNOWN_ERROR;
            }
        }

        return STATUS_SUCCESS;
    }

    /* Function:        exchange_protocol_versions
     * Description:     Exchanges the protocol version strings.
     * 
     * TODO:            Implement UTF8 support.
     */
    int ssh_transport::exchange_protocol_versions()
    {
        string temp;
        uint32 count = 0;

        if(!socketLayer.writeRawLine(local_context.protocolVersion)) {
            cerr << "exchange_protocol_versions: Could not write the protocol version string" << endl;
            // just disconnect.
            force_disconnect();
            return CONNECTION_FAILED;
        }

        /* loop until we get the protocol version , but we don't want to continue forever */
        do {
            if((count++) >= MAX_NUM_COMMENTS) {
                // Close the socket.
                cerr << "The maximum number of comments has been exceeded" << endl; 
                socketLayer.disconnect();
                return CONNECTION_FAILED;
            }
            if(!socketLayer.readRawLine(temp, 255)) {   // read a maximum of 255 chars, including CR/LF
                cerr << "Error while reading the protocol version string" << endl;
                socketLayer.disconnect();
                return CONNECTION_FAILED;
            }
        } while(!protocol_version_string(temp));
        
        server_context.protocolVersion = temp;

        /* now check the server protocol version */
        if(!parse_protocol_version(server_context.protocolVersion, &serverInfo)) {
            cerr << "Could not parse the servers protocol version string" << endl;
            socketLayer.disconnect();
            return CONNECTION_FAILED;
        }
        
        if(!supported_version(serverInfo.version)) {
            cerr << "Does not support the server version" << endl;
            socketLayer.disconnect();
            return UNSUPPORTED_PROTOCOL_VERSION;
        }
        return STATUS_SUCCESS;
    }


    /* Finction:        ssh_transport::calculate_digest
     * Description:     Calculates the digest of the message.
     */
    bool ssh_transport::calculate_digest(ssh_msg * msg,             // the message      
                                        hmac_base * hmac,           // the integrity algorithm
                                        uint32 seq,         // the sequence number
                                        unsigned char * out,        // the output buffer
                                        uint32 block_size)  // the blocksize.
    {
        unsigned char * dst = (out == NULL ? msg->digest : out);
        uint32 digest_size;

        hmac->reinit();
        // need to convert the sequence number to network order (big endian)
        seq = __htonl32(seq);
        // update the hash with the sequence number
        hmac->update((unsigned char *)&seq, sizeof(uint32));
        // then the first block
        hmac->update((unsigned char *)&msg->hdr->s.hdr, sizeof(ssh_hdr));
        hmac->update(msg->payload, msg->payload_usage);
        hmac->finalize(dst, &digest_size);
        return true;
    }

    /* Function:        ssh_transport::encrypt_packet
     * Description:     Encrypts the entire packet, including the first block.
     */
    bool ssh_transport::encrypt_packet(ssh_msg & packet)
    {   
        if(serverDirection.cipher == NULL)
            return false;
        assert(((packet.payload_usage + sizeof(ssh_hdr)) % packet.block_size) == 0);
        // encrypt the entire packet.
        serverDirection.cipher->encrypt(packet.data,packet.data, packet.payload_usage + sizeof(ssh_hdr));;
        return true;
    }

    /* Function:            ssh_transport::decrypt_packet
     * Description:         Decrypts a packet. The first block in the message should
     *                      already be decrypted at this point.
     */
    bool ssh_transport::decrypt_packet(ssh_msg & packet)
    {
        if(serverDirection.cipher == NULL)
            return false;
        uint32 already_decrypted = (packet.block_size - sizeof(ssh_hdr));
        // decrypt the rest of the packet, the first block is already decrypted at this point.
        serverDirection.cipher->decrypt((unsigned char *)(packet.hdr + already_decrypted),
            (unsigned char *)(packet.hdr + already_decrypted), packet.payload_usage - already_decrypted);

        return true;
    }

    /* Function:        ssh_transport::initalize_algorithms
     * Description:     Initalizes the algorithms to use.
     */
    int ssh_transport::initalize_algorithms(algorithm_list & list, 
                                            Algorithms & client, 
                                            Algorithms & server, 
                                            kex_base ** kex,
                                            hostkey_base ** hostkey)
    {
        /* First create the algorithms to be used for the client direction */
        if(list.cipher_client_algorithm != "none") {
            if(!(client.cipher = create_cipher(list.cipher_client_algorithm))) {
                algorithm_cleanup();
                return ALGORITHM_INITALIZATION_FAILED;
            }
        } else
            if(m_notify) m_notify->OnWarning(L"The data will be sent to the client unencrypted.");

        if(list.hmac_client_algorithm != "none") {
            if(!(client.hmac = create_hmac(list.hmac_client_algorithm))) {
                algorithm_cleanup();
                return ALGORITHM_INITALIZATION_FAILED;
            }
        } else 
            if(m_notify) m_notify->OnWarning(L"No integrity control will be performed on the data.");

        /* initalize the algorithms for the server direction */
        if(list.cipher_server_algorithm != "none") {
            if(!(server.cipher = create_cipher(list.cipher_server_algorithm))) {
                algorithm_cleanup();
                return ALGORITHM_INITALIZATION_FAILED;
            }
        } else
            if(m_notify) m_notify->OnWarning(L"The data will be sent unencrypted to the server.");

        if(list.hmac_server_algorithm != "none") {
            if(!(server.hmac = create_hmac(list.hmac_server_algorithm))) {
                cerr << "Failed to create HMAC: " << list.hmac_server_algorithm << endl;
                algorithm_cleanup();
                return ALGORITHM_INITALIZATION_FAILED;
            }
        } else 
            if(m_notify) m_notify->OnWarning(L"No integrity control will be performed on the data.");

        /* create the keyexchange algorithm instance */
        if(!((*kex) = create_kex(list.keyexchange_algorithm))) {
            algorithm_cleanup();
            return ALGORITHM_INITALIZATION_FAILED;
        }
        
        /* Now create the hostkey */
        if(list.hostkey_algorithm != "none")
        {
            if(!((*hostkey) = create_hostkey(list.hostkey_algorithm))) {
                algorithm_cleanup();
                delete *kex;
                return ALGORITHM_INITALIZATION_FAILED;
            }
        } else
            if(m_notify) m_notify->OnWarning(L"No host authentication will be performed.");
        // return success.
        return STATUS_SUCCESS;
    }

    /* Function:        ssh_transport::algorithm_cleanup
     * Description:     Deletes the currently used algorithms and sets the pointers to zero.
     */
    void ssh_transport::algorithm_cleanup()
    {
        SAFE_DELETE(clientDirection.cipher)
        SAFE_DELETE(clientDirection.hmac)
        SAFE_DELETE(serverDirection.cipher)
        SAFE_DELETE(serverDirection.hmac)
    }

    /* Function:        ssh_transport::randomize_buffer
     * Description:     fills a buffer with random bytes.
     */
    void ssh_transport::randomize_buffer(unsigned char * dst, uint32 len)
    {
        for(uint32 index = 0;index < len;++index)
            dst[index] = rand() % 255;              // for now.
    }

    /* Function:            ssh_transport::setLastError
     * Description:         Sets the last error.
     */
    void ssh_transport::setLastError(int code, const char * reason)
    {
        m_error     = code;
        m_errorStr  = reason;
    }

    /* Function:        ssh_transport::setAlgorithms
     * Description:     Sets the current algorithms and performs the required cleanup
     *                  on the old ones.
     */
    void ssh_transport::setAlgorithms(Algorithms & client, Algorithms & server)
    {
        /* Delete the old ones */
        SAFE_DELETE(clientDirection.cipher)
        SAFE_DELETE(clientDirection.hmac)
        SAFE_DELETE(serverDirection.cipher)
        SAFE_DELETE(serverDirection.hmac)
        
        /* Set the new ones */
        clientDirection.cipher  = client.cipher;    
        clientDirection.hmac    = client.hmac;      
        serverDirection.cipher  = server.cipher;    
        serverDirection.hmac    = server.hmac;      

        /* must null this pointers */
        client.cipher   = NULL;
        client.hmac     = NULL;
        server.cipher   = NULL;
        server.hmac     = NULL;
    }

    /* Function:        ssh_transport::getMessageType
     * Description:     returns the type of the read message.
     */
    unsigned char ssh_transport::getMessageType()
    {
        return readState.msg.hdr->s.type;
    }

    /* Function:        ssh_transport::parse
     * Description:     Lets the channel parse the channel data.
     */
    int ssh_transport::parse(ssh::ssh_channel * channel, uint32 numbytes)
    { 
        // validate the supplied size.
        if(readState.position + numbytes > readState.total_size) {
            DebugPrint("Invalid data size")
            return FATAL_ERROR;
        }
        return channel->parse(readState.msg.payload + readState.position,numbytes);
    }

    /* Function:        ssh_transport::parse_extended
     * Description:     Lets the channel parse extended channel data.
     */
    int ssh_transport::parse_extended(ssh::ssh_channel * channel, uint32 numbytes, uint32 type)
    { 
        // validate the supplied size
        if(readState.position + numbytes > readState.total_size) {
            DebugPrint("Invalid data size")
            return FATAL_ERROR;
        }
        return channel->parse_extended(readState.msg.payload + readState.position,numbytes,type);
    }

};