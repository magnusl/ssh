#ifndef _HMAC_BASE_H_
#define _HMAC_BASE_H_

#include "types.h"

namespace ssh
{
    /* Class:           hmac_base
     * Description:     Baseclass for the keyed macs.
     */
    class hmac_base
    {
    public:
        virtual ~hmac_base() {
        }

        // initalizes the HMAC
        virtual bool init(unsigned char *)                      = 0;
        virtual uint32 getKeyLength()                       = 0;
        virtual bool update(unsigned char *, uint32)        = 0;
        virtual bool finalize(unsigned char *, uint32 *)    = 0;
        virtual bool reinit()                                   = 0;
        virtual uint32 getDigestLength()                    = 0;    
        static const char * DEFAULT_HMACS;

    };
};

#endif