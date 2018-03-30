#include "sftp_file_attributes.h"
#include <sys/types.h> 
#include <sys/stat.h>

namespace sftp
{
    /*
     *
     */
    bool sftp_file_attributes::set(const wchar_t * filename)
    {
        struct __stat64 buffer;
        if(_wstat64(filename, &buffer) != 0)
            return false;



    }
};