#ifndef _SFTP_FILE_BASE_H_
#define _SFTP_FILE_BASE_H_

#include "IStreamIO.h"
#include "types.h"

namespace sftp
{
    /* Class:           sftp_file_base
     * Description:     The baseclass for the different SFTP file implementations.
     */
    class sftp_file_base
    {
    public:
        // writes the file data to a stream
        virtual bool write(ssh::IStreamIO *)    = 0;
        // reads the file data from a stream
        virtual bool read(ssh::IStreamIO *)     = 0;
        // returns the size
        uint64 get_size() {return size;}
        // returns the access time in UTC
        const tm * access_time();
        // returns the modification time in UTC.
        const tm * modification_time();
        const std::wstring & get_filename() {return filename;}

    protected:
        uint64 size;
        std::wstring filename;
    };

};

#endif