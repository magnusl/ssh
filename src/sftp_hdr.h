#ifndef _SFTP_HDR_H_
#define _SFTP_HDR_H_

namespace sftp
{

#if defined(_MSC_VER)
#pragma pack(push, 1)

    /* Struct:          sftp_hdr
     * Description:     The SFTP packet header,
     */
    struct sftp_hdr
    {
        uint32          length;
        unsigned char   type;
        uint32          request_id;
    };

#pragma pack(pop)
#else
#error Compiler not supported 
#endif
};

#endif