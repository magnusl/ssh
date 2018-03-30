/* File:            sftp.cpp
 * Description: 
 * Author:  
 *
 *****************************************************************************/

#include "sftp.h"
#include "sftp_messages.h"
#include <iostream>
#include "ArrayStream.h"
#include <assert.h>
#include "common.h"
#include "sftp_open_dir_v3.h"
#include "sftp_dir_request_v3.h"
#include "sftp_close_request.h"
#include "sftp_impl_v3.h"

#define SFTP_SEND_BUFFER_SIZE       48000
#define SFTP_READ_BUFFER_SIZE       48000
#define MAX_SFTP_PAYLOAD            (SFTP_READ_BUFFER_SIZE - sizeof(sftp_hdr))

using namespace std;

namespace sftp
{
    /* Function:        sftp_connection::sftp_connection
     * Description:     Constructor, initalizes the SFTP connection.
     */
    sftp_connection::sftp_connection(ssh::ssh_channel * channel, sftp_raw_notify * notify) : m_channel(channel),
        m_notify(notify), request_id(0)
    {
        send_buffer  = new unsigned char[SFTP_SEND_BUFFER_SIZE];
        read_buffer  = new unsigned char[SFTP_READ_BUFFER_SIZE];
        read_hdr     = reinterpret_cast<sftp_hdr *>(read_buffer);
        send_hdr     = reinterpret_cast<sftp_hdr *>(send_buffer);
        send_payload = send_buffer + sizeof(sftp_hdr);

        m_sendstate = STATE_NO_PACKET;
        m_readstate = STATE_NO_PACKET;

        // set the initalization variables
        init_read_state = SFTP_READING_INIT_HEADER;
        init_send_state = SFTP_STATE_INIT;
        init_state = SFTP_STATE_INIT;
        init_bytes_sent = 0;
        init_bytes_read = 0;
        version_packet.length = __htonl32(SFTP_VERSION_HDR_SIZE);
        version_packet.type = SSH_FXP_INIT;
        version_packet.version = __htonl32(3);
        version = 3;
        impl = new sftp_impl_v3();

        //read_buff_len = SFTP_READ_BUFFER_SIZE;
        //send_buff_len = SFTP_SEND_BUFFER_SIZE;
    }

    /* Function:        sftp_connection::update()
     * Description:     Updates the connection. Works in a nonblocking way.
     */
    int sftp_connection::update()
    {   
        int nret;
        {
            if(m_sendstate == STATE_NO_PACKET) {
                if(!request_queue.empty()) {
                    /* handle a request */
                    sftp_request * request = request_queue.front();
                    request_queue.pop();
                    // write the request to the packet.
                    uint32 id = request_id++;
                    ssh::ArrayStream as(&send_hdr->type, MAX_SFTP_PAYLOAD);

                    cerr << "Writing the request to the packet" << endl;
                    nret = request->write_request(&as, id);
                    if(nret < 0) {
                        cerr << "A error occurred when writing the request" << endl;
                        return nret;
                    }
                    send_size = as.usage();
                    // now add the request to the waiting list.
                    pending_requests[id] = request;
                    nret = sendpacket_nonblock();
                    if(nret < 0) {
                        cerr << "Error while sending the packet" << endl;
                        return nret;
                    }
                }

            } else {
                // continue to send the packet
                nret = sendpacket_nonblock();
                if(nret < 0) {
                    cerr << "Error while continuing sending the packet" << endl;
                    return nret;
                }
            }
            nret = readpacket_nonblock();
            if(nret < 0) {
                cerr << "Error while reading a packet" << endl;
                return nret;
            }
            if(nret == PACKET_COMPLETE) {
                // read a packet, now handle it.
                return handle_packet();
            }
            return STATUS_SUCCESS;
        }
    }

    /* Function:        sftp_connection::sendpacket_nonblock
     * Description:     Writes a packet to the channel buffer.
     */
    int sftp_connection::sendpacket_nonblock()
    {
        int nret;
        if(m_sendstate == STATE_NO_PACKET) {
            send_packet_size = send_size + sizeof(uint32);
            send_hdr->length = __htonl32(send_size);
            m_sendstate = STATE_SENDING_PACKET;
            bytes_sent = 0;
            return PACKET_PENDING;
        } else if(m_sendstate == STATE_SENDING_PACKET) {
            nret = m_channel->writeBytes(send_buffer + bytes_sent, send_packet_size - bytes_sent);
            if(nret < 0) {
                cerr << "Could not write data to the packet, error: " << nret << endl;
                return nret;
            }
            bytes_sent += nret;
            if(bytes_sent == send_packet_size) {
                m_sendstate = STATE_NO_PACKET;
                return PACKET_SENT;
            }
            return PACKET_PENDING;
        } else {
            cerr << "Invalid send state" << endl;
            return FATAL_ERROR;
        }
    }

    /* Function:        sftp_connection::readpacket_nonblock
     * Description:     Reads a packet from the channel buffer.
     */
    int sftp_connection::readpacket_nonblock()
    {
        if(!m_channel->data_available())
            return (m_readstate == STATE_NO_PACKET ? NO_PACKET : PACKET_PENDING);
        int nret;
        if(m_readstate == STATE_NO_PACKET) {
            m_readstate = STATE_READING_HEADER;
            // haven't read anything yet.
            bytes_read = 0;
        }
        
        if(m_readstate == STATE_READING_HEADER) {
            /* read the SFTP header */
            if(bytes_read < sizeof(sftp_hdr)) {
                // read the header.
                nret = m_channel->readBytes(reinterpret_cast<unsigned char *>(read_hdr) + bytes_read,
                    sizeof(sftp_hdr) - bytes_read);
                if(nret < 0) return nret;
                bytes_read += nret;
                if(bytes_read == sizeof(sftp_hdr)) {
                    // read the header.
                    m_readstate = STATE_READING_PAYLOAD;
                    read_hdr->length = __ntohl32(read_hdr->length);
                    if(read_hdr->length > SFTP_READ_BUFFER_SIZE) {
                        cerr << "The SFTP packet is to large" << endl;
                        return FATAL_ERROR;
                    }
                } else {
                    return PACKET_PENDING;
                }
            }
        }
        
        if(m_readstate == STATE_READING_PAYLOAD) {
            /* Read the packet payload */
            nret = m_channel->readBytes(reinterpret_cast<unsigned char *>(read_hdr) + bytes_read,
                read_hdr->length - bytes_read + sizeof(uint32));
            if(nret < 0) return nret;
            bytes_read += nret;
            if(bytes_read == (read_hdr->length + sizeof(uint32))) {
                // read the entire packet
                read_hdr->request_id = __ntohl32(read_hdr->request_id);
                m_readstate = STATE_NO_PACKET;
                return PACKET_COMPLETE;
            }
        } else {
            /* invalid state */
            return FATAL_ERROR;
        }
        return STATUS_FAILURE;
    }

    /* Function:        sftp_connection::handle_packet
     * Description:     Handles a received message.
     */
    int sftp_connection::handle_packet()
    {
        cerr << "Got a reply for request: " << read_hdr->request_id << endl;
        std::map<uint32, sftp_request *>::iterator it = pending_requests.find(read_hdr->request_id);
        if(it == pending_requests.end())
            return FATAL_ERROR;     // for now.
        if(it->second == NULL)
            return FATAL_ERROR;
        sftp_request * request = it->second;
        // erase it from the list
        pending_requests.erase(it);

        // let the request parse the reply
        ssh::ArrayStream stream(read_buffer + sizeof(sftp_hdr), read_hdr->length - 5);
        int nret = request->parse_reply(read_hdr->type, &stream);
        if(nret < 0) {
            delete request;
            cerr << "Failed to process the request reply" << endl;
            return FATAL_ERROR;
        } else if(nret == SFTP_REQUEST_COMPLETE) {
            // the request is complete.
            delete request;
            std::cerr << "Request complete" << endl;
            return STATUS_SUCCESS;
        } else if(nret == SFTP_REQUEST_PENDING) {
            // the request is pending, so add it to the request list
            request_queue.push(request);
            std::cerr << "Request not complete uet" << endl;
            return STATUS_SUCCESS;
        }
        assert(false);
        return FATAL_ERROR;
    }
};