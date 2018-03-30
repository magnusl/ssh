#ifndef _HMAC_OPENSSL_H_
#define _HMAC_OPENSSL_H_

#include "hmac_base.h"
#include "types.h"
#include <openssl/hmac.h>

namespace ssh
{
    /* Class:           hmac_openssl
     * Description:     The baseclass for the OpenSSL based keyd MACs.
     */
    class hmac_openssl : public hmac_base
    {
    public:
        ~hmac_openssl() {
            HMAC_CTX_cleanup(&ctx);
        }
        // initalizes the HMAC
        bool init(unsigned char *);
        bool update(unsigned char *, uint32);
        bool finalize(unsigned char *, uint32 *);
        bool reinit();
        
        virtual uint32 getKeyLength()           = 0;
        virtual uint32 getDigestLength()        = 0;

        static const char * DEFAULT_HMACS;
    protected:
        const EVP_MD * m_digest;
        HMAC_CTX ctx;
    };
};

#endif