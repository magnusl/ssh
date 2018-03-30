#ifndef _SFTP_DIR_REQUEST_H_
#define _SFTP_DIR_REQUEST_H_

#include "sftp_request.h"
#include "sftp_directory.h"
#include "types.h"
#include "ArrayStream.h"
#include "sftp_handle.h"
#include "IStreamIO.h"

namespace sftp
{
    /* Class:           sftp_dir_request
     * Description:     Used to obtain a directory listing.
     */
    class sftp_dir_request : public sftp_request
    {
    public:
        sftp_dir_request(sftp_raw_notify * notify, sftp_handle * handle, sftp_directory * dir) : sftp_request(notify), 
            m_dir(dir) , m_handle(handle) {
        }

        virtual int parse_reply(unsigned char, ssh::IStreamIO *) = 0;
        virtual int write_request(ssh::IStreamIO *, uint32);

    protected:
        sftp_directory   *  m_dir;          // the directory.
        uint32              m_state;        // the current state
        sftp_handle     *   m_handle;
    };
};

#endif