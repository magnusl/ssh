#ifndef _SSH_MSG_H_
#define _SSH_MSG_H_

#include "ssh_hdr.h"
#include "definitions.h"

namespace ssh
{

    /* struct:          ssh_msg
     * description:     A ssh message.
     */
    class ssh_msg
    {
    public:
        union packet_hdr{
            struct // a better alternative maybe?
            {
                ssh_hdr hdr;    
                unsigned char type;
            } s;
            unsigned char first_block[MAX_BLOCK_SIZE];  
        };

        packet_hdr * hdr;
        unsigned char * payload;
        unsigned char * data;                       // the base pointer.

        uint32 size;                            // the data.
        unsigned char digest[MAX_DIGEST_LENGTH];    // the digest
        uint32 block_size;                  // the block size used

        uint32 payload_usage;
    };
};
#endif