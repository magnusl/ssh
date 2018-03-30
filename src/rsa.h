/* File:            rsa.h
 * Author:          Magnus Leksell
 * Description:     Implements the SSH2 RSA host verification process.
 *
 * Copyright © 2006-2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/
#ifndef _RSA_H_
#define _RSA_H_

#include "hostkey_base.h"
#include "types.h"
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include "common.h"
#include "IStreamIO.h"

namespace ssh
{
    /* Class:           RSA_hostkey
     * Description:     RSA host verification.
     */
    class RSA_hostkey : public hostkey_base
    {
    public:
        RSA_hostkey();
        ~RSA_hostkey();
        // parses the keyblob.  
        bool parse_keyblob(unsigned char *, uint32);    
        // parses the keyblob.  
        bool parse_signature(unsigned char *, uint32);
        // verifies the host    
        int verify_host(unsigned char *, uint32);   
        // returns the parsed host key.
        bool hash_keyblob(const char *, byte *, uint32 *) const;
        const char * identifier() const {return "rsa";}
        size_t numbits() const;
    protected:
        unsigned char * signature;
        uint32 sign_len;
        RSA * rsa;
    };
};
#endif