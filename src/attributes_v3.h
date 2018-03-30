#ifndef _ATTRIBUTES_V3_H_
#define _ATTRIBUTES_V3_H_

#include "IStreamIO.h"
#include <string>
#include <list>
#include "file_attributes.h"
#include <vector>
#include "types.h"

namespace sftp
{
    /* Class:           attributes_v3
     * Description:     File attributes in the SFTP version 3 protocol.
     */
    class attributes_v3 : public file_attributes
    {
    public:
        attributes_v3() {flags = 0;}
        // writes the attributes to a stream.
        bool write(ssh::IStreamIO *) const;
        bool read(ssh::IStreamIO *);

        int getVersion() {return 3;}            // returns the version
        int setAttributes(const wchar_t *);     // loads the attributes from a file.

        bool getCreateTime(uint64 &)        const;          
        bool getModificationTime(uint64 &)  const;
        bool getAccessTime(uint64 &)        const;
        bool getFileSize(uint64 &)          const;

    private:
        uint32 flags,               // specifies what fields is present
                uid,                // user identifier  
                gid,                // group identifier
                permissions,        // posix bit mask for permissions
                atime,              // access time
                mtime,              // modification time
                extended_count;     // the number of extensions.
        uint64 size;
        std::list<std::pair<std::string, std::string> > extensions;

    };

};

#endif