/* File:            attributes_v4
 * Author:          Magnus Leksell
 * 
 * Copyright © 2007 Magnus Leksell,all rights reserved.
 *****************************************************************************/

#include "attributes_v4.h"
#include "attrs.h"
#include "definitions.h"
#include <iostream>
#include <list>
#include <string>
#include <assert.h>

/*
 * Some required windows specifc stuff.
 */
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
    int attributes_v4::setAttributes(const wchar_t * filename)
    {
#ifdef _WIN32
        /*
         * Windows specific stuff
         */
        struct _stati64 buf;
        if(_wstati64(filename, &buf) != 0) {
            wcerr << L"Could not retrive the file attributes for the file" << filename << endl;
            return FATAL_ERROR;
        }

        flags = 0;
        // Now get the attributes
        size = buf.st_size;         flags |= SSH_FILEXFER_ATTR_SIZE;
        atime = buf.st_atime;       flags |= SSH_FILEXFER_ATTR_ACCESSTIME;
        mtime = buf.st_mtime;       flags |= SSH_FILEXFER_ATTR_MODIFYTIME;
        createtime = buf.st_ctime;  flags |= SSH_FILEXFER_ATTR_CREATETIME;
#else
        /*
         * Posix
         */

#endif
    
        return STATUS_SUCCESS;
    }


    /* Function:        attributes_v3::read
     * Description:     Reads the file attributes from a stream
     */
    bool attributes_v4::read(ssh::IStreamIO * stream)
    {
        // read the flags and the type
        if(!stream->readInt32(flags) || !stream->readByte(type))
            return false;

        // read the file size
        if(flags & SSH_FILEXFER_ATTR_SIZE) {
            // the size is present
            if(!stream->readInt64(size)) return false;
        }

        // read the owner and group
        if(flags & SSH_FILEXFER_ATTR_OWNERGROUP) {
            if(!stream->readWideString(owner) || !stream->readWideString(group))
                return false;
        }

        // read the permissions
        if(flags & SSH_FILEXFER_ATTR_PERMISSIONS) {
            if(!stream->readInt32(permissions)) return false;
        }

        // read the access time
        if(flags & SSH_FILEXFER_ATTR_ACCESSTIME) {
            if(!stream->readInt64(atime) || 
                (flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES ? stream->readInt32(atime_nseconds) : true))
                return false;
        }
        
        // Read the create time
        if(flags & SSH_FILEXFER_ATTR_CREATETIME) {
            if(!stream->readInt64(createtime) || 
                (flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES ? stream->readInt32(createtime_nseconds) : true))
                return false;
        }

        // Read the modification time
        if(flags & SSH_FILEXFER_ATTR_MODIFYTIME) {
            if(!stream->readInt64(mtime) || 
                (flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES ? stream->readInt32(mtime_nseconds) : true))
                return false;
        }

        // Read the access control list
        if(flags & SSH_FILEXFER_ATTR_ACL) {
            if(!m_acl.read(stream)) {
                return false;
            }
        }

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
    bool attributes_v4::write(ssh::IStreamIO * stream) const
    {
        if(!stream->writeInt32(flags) || !stream->writeByte(type))
            return false;

        // write the file size
        if(flags & SSH_FILEXFER_ATTR_SIZE) {
            // the size is present
            if(!stream->writeInt64(size)) return false;
        }

        // write the owner and group
        if(flags & SSH_FILEXFER_ATTR_OWNERGROUP) {
            if(!stream->writeWideString(owner) || !stream->writeWideString(group))
                return false;
        }

        // write the permissions
        if(flags & SSH_FILEXFER_ATTR_PERMISSIONS) {
            if(!stream->writeInt32(permissions)) return false;
        }

        // write the access time
        if(flags & SSH_FILEXFER_ATTR_ACCESSTIME) {
            if(!stream->writeInt64(atime) || 
                (flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES ? stream->writeInt32(atime_nseconds) : true))
                return false;
        }
        
        // write the create time
        if(flags & SSH_FILEXFER_ATTR_CREATETIME) {
            if(!stream->writeInt64(createtime) || 
                (flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES ? stream->writeInt32(createtime_nseconds) : true))
                return false;
        }

        // write the modification time
        if(flags & SSH_FILEXFER_ATTR_MODIFYTIME) {
            if(!stream->writeInt64(mtime) || 
                (flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES ? stream->writeInt32(mtime_nseconds) : true))
                return false;
        }

        // write the access control list
        if(flags & SSH_FILEXFER_ATTR_ACL) {
            if(!m_acl.write(stream)) {
                return false;
            }
        }

        // Write the extensions,
        if(flags & SSH_FILEXFER_ATTR_EXTENDED) {
            if(!stream->writeInt32(static_cast<uint32>(extensions.size())))
                return false;
            // read all the extensions.
            for(std::list<std::pair<std::string, std::string> >::const_iterator it = extensions.begin();
                it != extensions.end();
                it++)
            {
                if(!stream->writeString(it->first) || !stream->writeString(it->second)) 
                    return false;
            }
        }
        return true;
    }

    /*
     *
     */
    bool attributes_v4::getFileSize(uint64 & _size) const
    {
        if(flags & SSH_FILEXFER_ATTR_SIZE) {
            _size = size;
            return true;
        }
        return false;
    }

    /*
     *
     */
    bool attributes_v4::getCreateTime(uint64 & _ctime) const
    {
        if(flags & SSH_FILEXFER_ATTR_CREATETIME) {
            _ctime = createtime;
            return true;
        }
        return false;
    }

    /*
     *
     */
    bool attributes_v4::getModificationTime(uint64 & _mtime) const
    {
        // write the modification time
        if(flags & SSH_FILEXFER_ATTR_MODIFYTIME) {
            _mtime = mtime;
            return true;
        }
        return false;
    }

    /*
     *
     */
    bool attributes_v4::getAccessTime(uint64 & _atime) const
    {
        if(flags & SSH_FILEXFER_ATTR_ACCESSTIME) {
            _atime = atime;
            return true;
        }
        return false;
    }
}