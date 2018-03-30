#ifndef _SFTP_FILE_H_
#define _SFTP_FILE_H_

#include <string>
#include "types.h"
#include "ATTRS.h"

namespace sftp
{
    /*
     *
     */
    class sftp_file
    {
    public:
        std::wstring m_filename;            // the filename
        ATTRS m_attributes;                 // the attributes
    };
};

#endif