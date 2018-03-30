/* File:            packet_functions
 * Description:     Contains all the functions relating to the packets in
 *                  the SSH transport layer.
 * Author:          Magnus Leksell
 * Copyright 2006-2007 © Magnus Leksell, all rights reserved.
 *****************************************************************************/
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <assert.h>

#include "ssh_transport.h"
#include "definitions.h"
#include "sshmessages.h"
#include "common.h"

void dump(char * name, unsigned char * data, uint32 len);

namespace ssh
{

    using namespace std;

    /* Function:        ssh_transport::newPacket
     * Description:
     */
    void ssh_transport::newPacket()
    {
        // reset the write position.
        sendState.position = 0; 
        sendState.state = STATE_NO_PACKET;
        sendState.msg.payload_usage = 0;
    }

    /* Function:            ssh_transport::readpacket_nonblock
     * Description:         Sends a packet in nonblocking mode. The contents of 
     *                      the packet should be in readState.msg.
     *
     * Bugfixes and updates:    
     *      - 2007/06/13, rewrote the entire function to a more suitable
     *                    implementation
     *      - 2007/06/23, Yet a new implementation. The entire packet is stored in
     *                    a linear way and is read in two read calls, one for the
     *                    first block and one for the rest (payload, padding, 
     *                    integrity digest).
     *****************************************************************************/
    int ssh_transport::readpacket_nonblock()
    {
        int nret;
        /* return if there isn't any data to read */
        if(!socketLayer.data_available())
            return (readState.state != STATE_NO_PACKET ? PACKET_PENDING : NO_PACKET);

        if(readState.state == STATE_NO_PACKET)
        {
            readState.bytes_transfered = 0;     // haven't read anything.
            readState.state = STATE_READING_FIRSTBLOCK;
            /* set the block size */
            readState.blocksize = (clientDirection.cipher == NULL ? 8 : clientDirection.cipher->getBlockSize());
            /* set the digest length */
            readState.digest_length = (clientDirection.hmac == NULL ? 0 : clientDirection.hmac->getDigestLength());
            readState.position = 0;
        }
        if(readState.state == STATE_READING_FIRSTBLOCK)
        {
            /* currently reading the first block */
            assert(readState.bytes_transfered < readState.blocksize);
            nret = socketLayer.read(readState.msg.data+readState.bytes_transfered ,readState.blocksize-readState.bytes_transfered );
            if(nret <= 0) {
                // a error occurred.
                return nret;
            }
            readState.bytes_transfered += nret;
            readState.total_bytes_transfered += nret;
            // have we read the first block?
            if(readState.bytes_transfered < readState.blocksize)
                return PACKET_PENDING;

            // the first block has been read, need to decrypt it
            if(clientDirection.cipher != NULL)
                clientDirection.cipher->decrypt(readState.msg.data, readState.msg.data, readState.blocksize);

            assert(sizeof(ssh_hdr) == 5);
            ssh_hdr * hdr = reinterpret_cast<ssh_hdr *>(readState.msg.hdr);
            
            // convert the size to the host byte order.
            hdr->packet_size = __ntohl32(hdr->packet_size);
            // save the header
            m_lastReadHdr = *hdr;
            // now validate the sizes.
            if((hdr->packet_size + hdr->padding_size + sizeof(ssh_hdr)) > MAX_PACKET_SIZE)
            {
                cerr << "The packet size is to large,'" << hdr->packet_size <<"' may be a decryption problem" << endl;
                return INVALID_PACKET_SIZE;
            }
            if(readState.msg.hdr->s.hdr.padding_size < 4) 
            {
                cerr << "Not enough padding, atleast 4 bytes is required " << endl;
                return INVALID_PADDING;
            }

            // calculate how much we should read.
            readState.total_size = hdr->packet_size + 4 +
                    ((clientDirection.hmac != NULL) ? clientDirection.hmac->getDigestLength() : 0);
            
            // have already read the first block
            readState.bytes_transfered = readState.blocksize;
            readState.state = STATE_READING_PACKET;

            if(readState.total_size == readState.bytes_transfered) {
                if(clientDirection.hmac != NULL) 
                    readState.state = STATE_VERIFY_INTEGRITY;
                else {
                    readState.seq.update();
                    readState.state = STATE_NO_PACKET;
                    return PACKET_COMPLETE;
                }
            }
            else {
                readState.state = STATE_READING_PACKET;
                return PACKET_PENDING;
            }
        } 
        if(readState.state == STATE_READING_PACKET)
        {
            /* reading the rest of the packet */
            unsigned char * dst = readState.msg.data + readState.bytes_transfered;
            nret = socketLayer.read(dst, readState.total_size - readState.bytes_transfered);
            if(nret <= 0) return nret;

            readState.bytes_transfered += nret;         // the number of bytes of this message transfered
            readState.total_bytes_transfered += nret;   // the _total_ number of bytes transfered.

            if(readState.bytes_transfered < readState.total_size) return PACKET_PENDING;        
            assert(((readState.total_size - readState.blocksize - readState.digest_length) % readState.blocksize) == 0);

            dst = readState.msg.data + readState.blocksize;
            readState.msg.payload_usage = readState.msg.hdr->s.hdr.packet_size - readState.msg.hdr->s.hdr.padding_size - 1;

            if(clientDirection.cipher != NULL) {
                // must decrypt the packet.
                clientDirection.cipher->decrypt(dst, dst,
                    readState.total_size - readState.blocksize - readState.digest_length);
            }
            if(clientDirection.hmac != NULL)  {
                // need to verify the integrity of the packet
                readState.state = STATE_VERIFY_INTEGRITY;
                readState.seq.update();
                readState.state = STATE_NO_PACKET;
                return PACKET_COMPLETE;
            }
            else {
                readState.seq.update();
                readState.state = STATE_NO_PACKET;
                return PACKET_COMPLETE;
            }
        }
        if(readState.state == STATE_VERIFY_INTEGRITY)
        {
            /* verifying the integrity of the message */
            uint32 seq = readState.seq.update();
            seq = __htonl32(seq);
            // verify the integrity of the packet.
            unsigned char digest[MAX_DIGEST_SIZE];
            uint32 dlen = MAX_DIGEST_SIZE;

            /* calculate the digest */
            clientDirection.hmac->reinit();
            clientDirection.hmac->update(reinterpret_cast<unsigned char *>(&seq), sizeof(uint32));
            clientDirection.hmac->update(readState.msg.data, readState.total_size - readState.digest_length);
            clientDirection.hmac->finalize(digest, &dlen);

            // compare the digests.
            unsigned char * remote_digest = readState.msg.data + readState.total_size - readState.digest_length;
            if(memcmp(remote_digest, digest, dlen) != 0) {
                cerr << "Fatal Error: The read packet is corrupt" << endl;
                return CORRUPT_PACKET;
            }
            else {
                readState.state = STATE_NO_PACKET;
                return PACKET_COMPLETE;
            }
        }
        assert(false);
        return FATAL_ERROR;
    }

    /* Function:        sendpacket_nonblock
     * Description:     Sends a packet in nonblocking mode.
     */
    int ssh_transport::sendpacket_nonblock()
    {
        int nret;
        if(sendState.state == STATE_NO_PACKET)
        {
            //ssh_msg & msg = sendState.msg;
            ssh_hdr & hdr = sendState.msg.hdr->s.hdr;

            /* Create a new packet */
            sendState.state = STATE_SENDING_PACKET;
            sendState.blocksize = (serverDirection.cipher == NULL ? 
                8 : serverDirection.cipher->getBlockSize());

            uint32 paddingSize = 4;
            uint32 mod = (sendState.msg.payload_usage + paddingSize + sizeof(ssh_hdr)) % sendState.blocksize;
            
            // might need som extra padding
            if(mod != 0)
                paddingSize += sendState.blocksize - mod;

            hdr.padding_size = (unsigned char) paddingSize;     // default padding for now.
            hdr.packet_size = sendState.msg.payload_usage + 1 + hdr.padding_size;
        
            // save the header
            m_lastSentHdr = hdr;

            // convert the size to BIG ENDIAN
            hdr.packet_size = __htonl32(hdr.packet_size);
            // update the sequence number
            uint32 seq = __htonl32(sendState.seq.update());
            // now add the padding
            unsigned char * padding = sendState.msg.payload + sendState.msg.payload_usage;
            unsigned char * digest = padding + paddingSize;
            randomize_buffer(padding, paddingSize);
            sendState.total_size = sendState.msg.payload_usage + paddingSize + sizeof(ssh_hdr);
            
            /* calculate the digest */
            if(serverDirection.hmac != NULL)  { 
                uint32 dlen = MAX_DIGEST_LENGTH;
                // add the digest after the padding.
                serverDirection.hmac->reinit();
                serverDirection.hmac->update(reinterpret_cast<unsigned char *>(&seq), sizeof(int32));
                // update the digest with the payload and the padding
                serverDirection.hmac->update(sendState.msg.data, sendState.msg.payload_usage + paddingSize + sizeof(ssh_hdr));
                // finalize the digest and add it behide the padding.
                serverDirection.hmac->finalize(digest, &dlen);
                sendState.total_size += dlen;
            }

            /* encrypt the packet */
            if(serverDirection.cipher != NULL) {
                // encrypt the payload and the padding.
                serverDirection.cipher->encrypt(sendState.msg.data, 
                    sendState.msg.data, sendState.msg.payload_usage + paddingSize + sizeof(ssh_hdr));
                //dump("encrypted", sendState.msg.data, sendState.msg.payload_usage + paddingSize + sizeof(ssh_hdr));
            }
            sendState.bytes_transfered = 0; 
        } 

        /* sends the packet */
        if(sendState.state == STATE_SENDING_PACKET)
        {
            //dump("output", sendState.msg.data, sendState.total_size);
            nret = socketLayer.write(sendState.msg.data + sendState.bytes_transfered, 
                sendState.total_size - sendState.bytes_transfered);
            if(nret <= 0) {
                return nret;
            }
            sendState.bytes_transfered += nret;         // the number of bytes transfered of this message
            sendState.total_bytes_transfered += nret;   // the _total_ number of bytes transfered.
            if(sendState.bytes_transfered < sendState.total_size) {
                return PACKET_PENDING;
            } else {
                sendState.state = STATE_NO_PACKET;
                return PACKET_SENT;
            }
        }
        assert(false);  // unknown state
        return FATAL_ERROR;
    }


    /* Function:        ssh_transport::exchangePackets
     * Description:     Exchanges two packets.
     */
    int ssh_transport::exchangePackets()
    {
        int nret;
        assert(sendState.state == STATE_NO_PACKET && readState.state == STATE_NO_PACKET);
        //readState.state = STATE_NO_PACKET;

        bool reading = true, writing = true;
        while(reading || writing)   
        {
            if(reading == true) {
                nret = readpacket_nonblock();
                if(nret < 0) {
                    // error
                    return nret;
                } else if(nret == PACKET_COMPLETE) 
                {
                    // packet is decrypted and integrity checked.
                    // what did we read? If we read a ignore message then we should wait for a new packet.
                    switch(readState.msg.hdr->s.type)
                    {
                    case SSH_MSG_IGNORE:
                        break;      // wait.
                    default:
                        reading = false;        // done reading.
                    }
                }
            }
            if(writing == true) {
                nret = sendpacket_nonblock();
                if(nret < 0) {
                    return nret;
                } else if(nret == PACKET_SENT) {
                    // packet sent to the server
                    writing = false;
                } else if(nret == DISCONNECTED) {
                    return nret;
                }
            }
        }
        assert(readState.position == 0);
        readState.position = 0;
        return STATUS_SUCCESS;
    }

    /* Function:        ssh_transport::exchangeAndExpect
     * Description:     Used for exchanging packets, when you are waiting for a 
     *                  specific response.
     */
    int ssh_transport::exchangeAndExpect(unsigned char type)
    {
        int nret = exchangePackets();
        if(nret != STATUS_SUCCESS)
            return nret;
        return (readState.msg.hdr->s.type == type ? STATUS_SUCCESS : WRONG_PACKET);
    }

    /* Function:        ssh_transport::readpacket
     * Description:     Waits until a packet has been read, skips ignore messages.
     */
    int ssh_transport::readpacket(int timeout)
    {
        int nret;
        time_t startTime = time(0);

        while(difftime(time(0) , startTime) < timeout)
        {
            nret = readpacket_nonblock();
            if(nret == PACKET_COMPLETE) {
                // finished reading the packet, now check the type
                switch(readState.msg.hdr->s.type)
                {
                case SSH_MSG_DISCONNECT:
                    // disconnected from the server.
                    return DISCONNECTED;
                case SSH_MSG_IGNORE:
                    continue;
                default:
                    return PACKET_COMPLETE;
                }
            }
            else if(nret < 0) {
                switch(nret)
                {
                case DISCONNECTED:
                    cerr << "Server closed the connection without sending a disconnect message" << endl;
                    return DISCONNECTED;
                default:
                    cerr << "A unknown error occurred while reading a packet" << endl;
                    // disconnect without send a disconnect message
                    force_disconnect();
                    return FATAL_ERROR;
                }
            }
        }
        cerr << "The read operation timed out" << endl;
        return TIMEOUT;
    }


    /* Function:        ssh_transport::sendpacket
     * Description:     Sends a packet in blocking mode.
     */
    int ssh_transport::sendpacket(int timeout)
    {
        // maybe I should introduce a timeout?
        int nret;
        while(true) {
            nret = sendpacket_nonblock();
            if(nret == PACKET_SENT) {
                return PACKET_SENT;
            }
            else if(nret == DISCONNECTED) {
                cerr << "Disocnnected from the server" << endl;
                return DISCONNECTED;
            } else if(nret < 0) {
                cerr << "A fatal error occurred while sending the packet" << endl;
                // disconnect without sending a message.
                force_disconnect();
                return FATAL_ERROR;
            }
        }
        assert(false); // should never be reached.
    }
};