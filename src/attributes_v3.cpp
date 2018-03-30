#include "attributes_v3.h"
#include "attrs.h"
#include <list>
#include <string>
#include "definitions.h"
#include <iostream>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <wchar.h>
#endif

using namespace std;

namespace sftp
{

    /* Function:        attributes_v3::setAttributes
     * Description:     Copies the file attributes from a file.
     */
    int attributes_v3::setAttributes(const wchar_t * filename)
    {
#ifdef _WIN32
        struct _stati64 buf;
        if(_wstati64(filename, &buf) != 0) {
            wcerr << L"Could not retrive the file attributes for the file" << filename << endl;
            return FATAL_ERROR;
        }

        flags = 0;
        // Now get the attributes
        size = buf.st_size;             flags |= SSH_FILEXFER_ATTR_SIZE;
        atime = (uint32) buf.st_atime;  flags |= SSH_FILEXFER_ATTR_ACMODTIME;
        mtime = (uint32) buf.st_mtime;  flags |= SSH_FILEXFER_ATTR_ACMODTIME;
#endif
        return STATUS_SUCCESS;
    }

    /* Function:        attributes_v3::read
     * Description:     Reads the file attributes from a stream
     */
    bool attributes_v3::read(ssh::IStreamIO * stream)
    {
        if(!stream->readInt32(flags)) {
            std::cerr << "Could not read the flags" << std::endl;
            return false;
        }
        if(flags & SSH_FILEXFER_ATTR_SIZE) {
            if(!stream->readInt64(size))  {
                std::cerr << "Could not read the file size" << std::endl;
                return false;
            }
        } else {
            size = 0;
        }

        /* read the uid and gid */
        if(flags & SSH_FILEXFER_ATTR_UIDGID)
            if(!stream->readInt32(uid) || !stream->readInt32(gid)) return false;
        /* read the permissions */
        if(flags & SSH_FILEXFER_ATTR_PERMISSIONS)
            if(!stream->readInt32(permissions)) return false;
        /* read the access time */
        if(flags & SSH_FILEXFER_ATTR_ACMODTIME)
            if(!stream->readInt32(atime)) return false;
        /* read the modification time */
        if(flags & SSH_FILEXFER_ATTR_ACMODTIME)
            if(!stream->readInt32(mtime)) return false;
        if(flags & SSH_FILEXFER_ATTR_EXTENDED) {
            if(!stream->readInt32(this->extended_count))
                return false;
            // read all the extensions.
            for(uint32 i = 0;i<this->extended_count;++i) {
                std::string str1, str2;
                // read the name and the value
                if(!stream->readString(str1) || !stream->readString(str2)) {
                    return false;
                }
                // add the extension
                extensions.push_back(std::pair<std::string, std::string>(str1, str2));
            }
        }
        return true;
    }

    /* Function:        attributes_v3::write
     * Description:     Writes the file attributes to the stream.
     */
    bool attributes_v3::write(ssh::IStreamIO * stream) const
    {
        if(!stream->writeInt32(flags))
            return false;
        if(flags & SSH_FILEXFER_ATTR_SIZE)
            if(!stream->writeInt64(size)) return false;
        /* read the uid and gid */
        if(flags & SSH_FILEXFER_ATTR_UIDGID)
            if(!stream->writeInt32(uid) || !stream->writeInt32(gid)) return false;
        /* read the permissions */
        if(flags & SSH_FILEXFER_ATTR_PERMISSIONS)
            if(!stream->writeInt32(permissions)) return false;
        /* read the access time */
        if(flags & SSH_FILEXFER_ATTR_ACMODTIME)
            if(!stream->writeInt32(atime)) return false;
        /* read the modification time */
        if(flags & SSH_FILEXFER_ATTR_ACMODTIME)
            if(!stream->writeInt32(mtime)) return false;
        if(flags & SSH_FILEXFER_ATTR_EXTENDED) {
            if(!stream->writeInt32(static_cast<uint32>(extensions.size()))) return false;
            /* write the extensions */
            for(list<pair<string,string> >::const_iterator it = extensions.begin();
                it != extensions.end();
                ++it) {
                if(!stream->writeString(it->first) || !stream->writeString(it->second))
                    return false;
            }
        }
        return true;
    }

    /* 
     *
     */
    bool attributes_v3::getCreateTime(uint64 & _ctime) const
    {
        return false;
    }

    /*
     *
     */
    bool attributes_v3::getModificationTime(uint64 & _mtime) const 
    {
        if(flags & SSH_FILEXFER_ATTR_ACMODTIME) {
            _mtime = mtime;
            return true;
        }
        return false;
    }

    /*
     *
     */
    bool attributes_v3::getAccessTime(uint64 & _atime) const
    {
        if(flags & SSH_FILEXFER_ATTR_ACMODTIME) {
            _atime = atime;
            return true;
        }
        return false;   // for now
    }

    /*
     *
     */
    bool attributes_v3::getFileSize(uint64 & fsize) const
    {
        if(flags & SSH_FILEXFER_ATTR_SIZE) {
            fsize = size;
            return true;
        }
        return false;
    }
};