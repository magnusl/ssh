/* File:            hmac_openssl.cpp
 * Description:
 * Author:          Magnus Leksell
 *
 * Copyright © 2006-2007 Magnus Leksell, all rights reserved
 *****************************************************************************/
#include "hmac_openssl.h"

namespace ssh
{

    /* Function:        hmac_openssl::init
     * Description:     Initalizes the hmac.
     */
    bool hmac_openssl::init(unsigned char * key)
    {
        HMAC_Init_ex(&ctx, key, getKeyLength(), m_digest,0);
        return true;
    }

    /* Function:        hmac_openssl::update
     * Description:     Updates the hmac with data.
     */
    bool hmac_openssl::update(unsigned char * data, uint32 len)
    {
        HMAC_Update(&ctx, data, len);
        return true;
    }

    /* Function:        hmac_openssl::finalize
     * Description:     Finalizes the digest.
     */
    bool hmac_openssl::finalize(unsigned char * digest, uint32 * len)
    {
        HMAC_Final(&ctx, digest, len);
        return true;
    }

    /* Function:        hmac_openssl::reinit
     * Description:     reinitalizes the hmac.
     */
    bool hmac_openssl::reinit()
    {
        HMAC_Init_ex(&ctx, 0, 0, 0,0);
        return true;
    }
};