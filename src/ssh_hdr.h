/* File:            ssh_hdr.h
 * Description: 
 * Author:          Magnus Leksell
 *
 * Copyright © 2006-2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/
#ifndef _SSH_HDR_H_
#define _SSH_HDR_H_

#include "types.h"

namespace ssh
{
#if defined(_MSC_VER)
#pragma pack(push, 1)
    struct ssh_hdr
    {
        uint32 packet_size;
        unsigned char padding_size;
    };
#pragma pack(pop)
#else
#if defined(__GNUC__)
    struct ssh_hdr
    {
        uint32 packet_size;
        unsigned char padding_size;
    } __attribute((packed));
#else
error Compiler not supported
#endif
#endif
};
#endif