#ifndef _SFTP_VERSION_3_H_
#define _SFTP_VERSION_3_H_

#include "sftp_impl.h"

namespace sftp
{
    /* Class:           sftp_version_3
     * Description:     Implementation of the SFTP version 3 protocol.
     */
    class sftp_version_3 : public sftp_impl
    {
    public:
        // creates a dir request
        sftp_dir_request * dir(sftp_handle *, sftp_directory *) const;
        // creates a open directory request
        sftp_open_dir * open_dir(const wchar_t *, sftp_handle *) const;
        // creates a open file request
        sftp_open_file * open_file(const wchar_t * path, sftp_handle * handle, 
            uint32 modes, const sftp_file_attributes &) const;
        // creates a close request
        sftp_close_request * close(sftp_handle * handle, bool) const;
        // creates a read data request
        sftp_read_data * read(sftp_handle *, unsigned char *, uint32) const;
        // reads data to a local file
        sftp_read_data * sftp_file_read(sftp_handle *, FILE *, uint32);
        // writes data from a file
        sftp_write_data * sftp_file_write(sftp_handle *, FILE *, uint32);
        // writes the file attributes to the stream.
        bool writeAttributes(sftp::sftp_file_attributes *, IStreamIO *);
    };
};


#endif