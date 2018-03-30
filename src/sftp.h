#ifndef _SFTP_H_
#define _SFTP_H_

#include "types.h"
#include "ssh_channel.h"
#include "sftp_directory.h"
#include "sftp_hdr.h"
#include <map>
#include "sftp_version.h"
#include "sftp_open_dir.h"
#include "sftp_definitions.h"
#include "sftp_impl.h"
#include "sftp_raw_notify.h"

#define STATE_READING_HEADER            1
#define SFTP_STATE_INIT                 2
#define SFTP_STATE_CLIENT_INIT          3
#define SFTP_STATE_SERVER_INIT          4

#define SFTP_INIT_IN_PROGRESS           5
#define SFTP_INIT_COMPLETE              6

#define SFTP_READING_INIT_HEADER        8
#define SFTP_READING_INIT_PAYLOAD       9

#define SFTP_VERSION_HDR_SIZE           (sizeof(sftp_version) - sizeof(uint32))

#define SFTP_NOTIFY_INITALIZED          1

namespace sftp
{
    /* Class:           sftp_connection
     * Description:
     */
    class sftp_connection
    {
    public:
        // the channel MUST have requested a sftp subsystem
        sftp_connection(ssh::ssh_channel *, sftp_raw_notify *);
        // initalizes the SFTP connection.
    protected:
        int readpacket_nonblock();
        int sendpacket_nonblock();
        int send_init_packet();
        int read_version_packet();

        int handle_packet();
        bool parse_version_packet();

        ssh::ssh_channel * m_channel;       // the ssh channel. // the callback function.
        unsigned char   *   send_buffer,
                        * read_buffer,          // the read buffer.
                        * send_payload;
        uint32 m_bufflen;               // the length of the buffer.
        
        uint32 version;
        int m_sendstate, m_readstate, init_state;       // the states.
        sftp_hdr * read_hdr, * send_hdr;                // the packet headers
        uint32 bytes_sent,                              // the number of bytes sent from the current packet
               bytes_read,                              // the number of bytes read from the current packet
               send_size,
               send_packet_size;            

        std::queue<sftp_request *>          request_queue;
        std::map<uint32, sftp_request   *>  pending_requests;
        uint32 request_id;

        /* initalization data */
        sftp_version    version_packet, server_version;
        uint32          init_send_state, init_read_state;
        uint32          init_bytes_sent, init_bytes_read, version_payload_size;

        sftp_impl * impl;
        sftp_raw_notify * m_notify;


    };
};

#endif