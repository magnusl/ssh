#ifndef _CONNECTIONSTATE_H_
#define _CONNECTIONSTATE_H_

#include "ssh_msg.h"
#include "sequence_number.h"

namespace ssh
{
    /* Struct:          ConnectionState
     *
     */
    struct ConnectionState
    {
    public:
        ssh_msg msg;                        // the current message
        int state;                      // the current state.
        uint32 blocksize,                   // the blocksize
            digest_length,                  // the digest length
            bytes_transfered;

        uint32 position, total_size;
        uint64 total_bytes_transfered;          // the total number of bytes sent
        sequence_number<uint32> seq;            // the 32 bit sequence number
    };
};
#endif