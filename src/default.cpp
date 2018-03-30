/* File:            default.cpp 
 * Description:     Sets the default algorithms to use.
 * Author:          Magnus Leksell
 *
 * Copyright © 2006-2007 Magnus Leksell, all rights reserved
 *****************************************************************************/

#include "cipher_base.h"
#include "hmac_base.h"
#include "kex_base.h"

namespace ssh
{
    /* The default algorithms */
    const char * cipher_base::DEFAULT_CIPHERS = "aes192-cbc,aes256-cbc,aes128-cbc";
    const char * hmac_base::DEFAULT_HMACS = "hmac-sha1";
    const char * kex_base::DEFAULT_KEXS = "diffie-hellman-group14-sha1,diffie-hellman-group1-sha1";
};