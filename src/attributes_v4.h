#ifndef _ATTRIBUTES_V4_H_
#define _ATTRIBUTES_V4_H_

#include "IStreamIO.h"
#include <string>
#include <list>
#include "file_attributes.h"
#include <vector>
#include "ACL.h"
#include "types.h"

namespace sftp
{
    /* Class:           attributes_v4
     * Description:     File attributes in the SFTP protocol version 4.
     */
    class attributes_v4 : public file_attributes
    {
    public:
        // Constructor
        attributes_v4() {flags = 0;}
        // writes the attributes to a stream.
        bool write(ssh::IStreamIO *) const;
        // read the attributes from a stream
        bool read(ssh::IStreamIO *);
        // returns the version
        int getVersion() {return 4;}            
        // loads the attributes from a file.
        int setAttributes(const wchar_t *);     
        // returns the size of the file.

        bool getFileSize(uint64 &) const;
        bool getCreateTime(uint64 &) const;         
        bool getModificationTime(uint64 &) const;
        bool getAccessTime(uint64 &) const;

        const std::wstring & getOwner() {return owner;}
        const std::wstring & getGroup() {return group;}
        const ACL & getACL()            {return m_acl;}
        uint32 getPermissions()         {return permissions;}

        uint64 getFileSize();
    private:
        unsigned char type;
        uint64  size,                   // the size
                atime,                  // the access time
                createtime,             // the creation time
                mtime;                  // the modification time
        uint32  flags,                  // the flags
                permissions,            // the permission mask
                atime_nseconds,         // access time, nanoseconds
                createtime_nseconds,    // creation time, nanoseconds
                mtime_nseconds,         // modification time, nanoseconds
                extended_count;         // the number of extensions.

        std::wstring    owner,          // the owner
                        group;          // the group

        ACL     m_acl;  // Access Control List?

        std::list<std::pair<std::string, std::string> > extensions;     // the extensions.

    };

};

#endif