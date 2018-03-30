#ifndef _SFTP_REQUEST_H_
#define _SFTP_REQUEST_H_

#include "types.h"
#include "IStreamIO.h"
#include "sftp_definitions.h"
#include "sftp_raw_notify.h"

namespace sftp
{
    class sftp_connection;

    /* Class:           sftp_request
     * Description:     The baseclass for the SFTP requests.
     */
    class sftp_request
    {
    public:
        virtual ~sftp_request() {}
        // processes data
        virtual int parse_reply(unsigned char, ssh::IStreamIO *) = 0;
        // writes the request, returns the number of bytes written.
        virtual int write_request(unsigned char * dst, uint32 len) = 0;
    };
};

#endif