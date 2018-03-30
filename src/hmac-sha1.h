/* File:            hmac-sha1.h
 * Description:     A keyed message authentication code is used to verify
 *                  the integrity of a packet. The implementation used here
 *                  wraps the functions implemented in the OpenSSL crypto 
 *                  library
 * Author:          Magnus Leksell
 * 
 * Copyright © 2006-2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/

#ifndef _HMAC_SHA1_H_
#define _HMAC_SHA1_H_

#include "hmac_base.h"
#include <openssl/hmac.h>
#include "definitions.h"
#include "hmac_openssl.h"

namespace ssh
{
    /* Class:           hmac_sha1
     * Description:     
     */
    class hmac_sha1 : public hmac_openssl
    {
    public:
        hmac_sha1() 
        {
            m_digest = EVP_sha1();
            HMAC_CTX_init(&ctx);
        }

        uint32 getKeyLength()       {return SHA1_KEY_LENGTH;}
        uint32 getDigestLength()    {return SHA1_DIGEST_LENGTH;}
    };
};
#endif