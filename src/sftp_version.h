#ifndef _SFTP_VERSION_H_
#define _SFTP_VERSION_H_

namespace sftp
{
    /* Struct:          sftp_version
     * Description:     the SFTP version packet.
     */
#if defined(_MSC_VER)
#pragma pack(push, 1)
    struct sftp_version
    {
        uint32          length;
        unsigned char   type;
        uint32          version;
    };
#pragma pack(pop)
#else
#error Compiler not supported
#endif
};

#endif