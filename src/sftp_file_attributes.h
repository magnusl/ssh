#ifndef _SFTP_FILE_ATTRIBUTES_H_
#define _SFTP_FILE_ATTRIBUTES_H_

#include "types.h"

namespace sftp
{
    /* Class:           sftp_file_attributes
     * Description:     Class for handling file attributes.
     */
    class sftp_file_attributes
    {
    public:
        sftp_file_attributes();
        // sets the file attributes from a file
        bool set(const wchar_t *);  

        uint64 m_size, m_atime, m_mtime, m_ctime;
        
        // writes the attributes to a stream
        bool write(IStreamIO * stream);

    };
};

#endif