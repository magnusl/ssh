#ifndef _SFTP_IMPL_H_
#define _SFTP_IMPL_H_

// Looks better with a define for thus =)
#define SFTP_VERSION_THREE      3
#define SFTP_VERSION_FOUR       4

#include "types.h"

namespace sftp
{
    /* Class:           sftp_impl
     * Description:     Baseclass for the SFTP version specific classes.
     */
    class sftp_impl
    {
    public:
        virtual int getVersion() = 0;
        virtual uint32 getPacketLimit() {return 32000;}
        virtual file_attributes * createAttributeInstance() = 0;
    };
};

#endif

