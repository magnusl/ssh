#ifndef _SFTP_DIR_REQUEST_V3_H
#define _SFTP_DIR_REQUEST_V3_H

#include "sftp_dir_request.h"
#include "sftp_file_version_3.h"

namespace sftp
{
    /* class:           sftp_dir_request_v3
     * Description:     Directory listing for the SFTP version 3 protocol.
     */
    class sftp_dir_request_v3 : public sftp_dir_request
    {
    public:
        sftp_dir_request_v3(sftp_raw_notify * notify, sftp_handle * handle, sftp_directory * dir) :
          sftp_dir_request(notify, handle, dir) {
          }

        int parse_reply(unsigned char,ssh::IStreamIO *);        // parses the server reply.
    };
};


#endif