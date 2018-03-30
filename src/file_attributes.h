#ifndef _FILE_ATTRIBUTES_H_
#define _FILE_ATTRIBUTES_H_

#include "IStreamIO.h"
#include <string>
#include "types.h"

namespace sftp
{
    /* Class:           file_attributes
     * Description:     Abstract baseclass for the file attributes
     */
    class file_attributes
    {
    public:
        virtual int getVersion() = 0;                       // returns the version (for downcasting)
        virtual int setAttributes(const wchar_t *) = 0;     // sets the attributes from a file.
        virtual int setAttributes(const std::wstring & filename) 
        {return setAttributes(filename.c_str());}
        virtual bool write(ssh::IStreamIO *) const = 0;     // writes the attributes to a stream
        virtual bool read(ssh::IStreamIO *) = 0;            // reads the attributes from a stream
        virtual bool getFileSize(uint64 &) const = 0;               // gets the file size
        virtual bool getCreateTime(uint64 &) const = 0;         
        virtual bool getModificationTime(uint64 &) const = 0;
        virtual bool getAccessTime(uint64 &) const = 0;
    };
};

#endif