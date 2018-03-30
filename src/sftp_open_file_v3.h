#ifndef _SFTP_OPEN_FILE_V3_H_
#define _SFTP_OPEN_FILE_V3_H_

#include "IStreamIO.h"
#include "sftp_open_file.h"
#include "attributes_v3.h"

namespace sftp
{
    /* Class:           sftp_open_file_v3
     * Description:     Used to open a remote file.
     */
    class sftp_open_file_v3 : public sftp_open_file
    {
    public:
        sftp_open_file_v3(sftp_raw_notify * notify, const wchar_t * filename, sftp_handle * handle, 
            uint32 flags,const sftp_file_attributes & attribs) 
            : sftp_open_file(notify, filename, handle, attribs) {
                pflags = flags;
        }
        
        // writes the request, returns the number of bytes written.
        int write_request(ssh::IStreamIO *, uint32);

    protected:
        attributes_v3   attributes;
        uint32 pflags;

    };
};

#endif
