#ifndef _ACL_H_
#define _ACL_H_

#include <list>
#include "types.h"
#include "IStreamIO.h"
#include <assert.h>

namespace sftp
{
    /* Sstruct:         ACE
     * Description:     Access Control Element.
     */
    struct ACE
    {   
        /*
                uint32   ace-type
                uint32   ace-flag
                uint32   ace-mask
                string   who [UTF-8]
        */
        uint32 ace_type, ace_flag, ace_mask;
        std::wstring who;
    };

    /* Struct:          ACL
     * Description:     A Access Control List.
     */
    struct ACL
    {
        uint32 acl_flags, ace_count;
        std::list<ACE> aces;
        // read the ACL from a stream
        bool read(ssh::IStreamIO * stream)
        {   
            uint32 length;
            if(!stream->readInt32(length) || !stream->readInt32(ace_count))
                return false;

            for(uint32 index = 0;index < ace_count;++index)
            {
                ACE ace;
                if(!stream->readInt32(ace.ace_type) || 
                    !stream->readInt32(ace.ace_flag) || 
                    !stream->readInt32(ace.ace_mask) ||
                    !stream->readWideString(ace.who))
                {
                    return false;
                }
                aces.push_back(ace);
            }
            return true;
        }

        /* Function:        write   
         * Description:     Writes the ACL to the string.
         */
        bool write(ssh::IStreamIO * stream) const
        {
            assert(false);
            return false;
        }
    };
};

#endif