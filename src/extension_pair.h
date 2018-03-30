#ifndef _EXTENSION_PAIR_H_
#define _EXTENSION_PAIR_H_

#include "types.h"

namespace sftp
{
    /* Struct:      extension_pair
     *
     */
    struct extension_pair
    {
        unsigned char * name, * data;
        uint32 name_len, data_len;
    };
};

#endif