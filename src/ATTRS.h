#ifndef _ATTRS_H_
#define _ATTRS_H_

#include "ACL.h"
#include "extension_pair.h"

#define SSH_FILEXFER_ATTR_SIZE              0x00000001
#define SSH_FILEXFER_ATTR_UIDGID            0x00000002
#define SSH_FILEXFER_ATTR_PERMISSIONS       0x00000004
#define SSH_FILEXFER_ATTR_ACCESSTIME        0x00000008
#define SSH_FILEXFER_ATTR_ACMODTIME         0x00000008
#define SSH_FILEXFER_ATTR_CREATETIME        0x00000010
#define SSH_FILEXFER_ATTR_MODIFYTIME        0x00000020
#define SSH_FILEXFER_ATTR_ACL               0x00000040
#define SSH_FILEXFER_ATTR_OWNERGROUP        0x00000080
#define SSH_FILEXFER_ATTR_SUBSECOND_TIMES   0x00000100
#define SSH_FILEXFER_ATTR_BITS              0x00000200
#define SSH_FILEXFER_ATTR_ALLOCATION_SIZE   0x00000400
#define SSH_FILEXFER_ATTR_TEXT_HINT         0x00000800
#define SSH_FILEXFER_ATTR_MIME_TYPE         0x00001000
#define SSH_FILEXFER_ATTR_LINK_COUNT        0x00002000
#define SSH_FILEXFER_ATTR_UNTRANSLATED_NAME 0x00004000
#define SSH_FILEXFER_ATTR_CTIME             0x00008000
#define SSH_FILEXFER_ATTR_EXTENDED          0x80000000

namespace sftp
{
    /* struct:          ATTRS
     * Description:     File Attributes
     */
    struct ATTRS
    {
        uint32 valid_attribute_flags, 
            permissions, 
            atime_nseconds, 
            createtime_nseconds,
            mtime_nseconds,
            ctime_nseconds,
            attrib_bits,
            attrib_bits_valid,
            link_count,
            extended_count;
        uint64 size,
            allocation_size,
            atime,
            createtime,
            mtime,
            ctime;
        unsigned char type,
            text_hint;
        std::wstring owner, 
            group, 
            mime_type;
        std::string untranslated_name;
        ACL acl;
        std::list<extension_pair> extensions;
    };
};

#endif