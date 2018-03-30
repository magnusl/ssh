#ifndef _SFTP_OPEN_DIR_V3_H_
#define _SFTP_OPEN_DIR_V3_H

#include "sftp_open_dir.h"

namespace sftp
{
    /* Class:           sftp_open_dir_v3
     * Description:     
     */
    class sftp_open_dir_v3 : public sftp_open_dir
    {
    public:
        sftp_open_dir_v3(sftp_raw_notify * notify, const wchar_t * dir, sftp_handle * handle) :
          sftp_open_dir( notify,dir, handle) {
        }
    };
};

#endif