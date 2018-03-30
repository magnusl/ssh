#ifndef _SFTP_OPEN_DIR_H_
#define _SFTP_OPEN_DIR_H_

#include "sftp_request.h"
#include "sftp_handle.h"
#include <string>
#include "IStreamIO.h"

namespace sftp
{
    /* Class:           sftp_open_dir
     * Description:     Used to open a directory for reading.
     */
    class sftp_open_dir : public sftp_request
    {
    public:
        sftp_open_dir(sftp_raw_notify *, const wchar_t * dir, sftp_handle *);
        int parse_reply(unsigned char,  ssh::IStreamIO * );
        int write_request(ssh::IStreamIO *, uint32);
    protected:
        sftp_handle * m_handle;
        std::wstring m_path;
    };
};

#endif