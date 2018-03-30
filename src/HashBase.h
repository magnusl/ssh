#ifndef _HASHBASE_H_
#define _HASHBASE_H_

#include <openssl/evp.h>
#include "types.h"

namespace ssh
{
    /* Function:        HashBase
     * Description:     Baseclass for the hash algorithms
     */
    class HashBase
    {
    public:
        HashBase(const EVP_MD * evp) : m_evp(evp) {
            // initalize the digest.
            EVP_DigestInit(&md, evp);
        }
        virtual ~HashBase() {
            // Cleanup
            EVP_MD_CTX_cleanup(&md);
        }
        // updates the digest.
        void update(const unsigned char * src, int num) {
            EVP_DigestUpdate(&md, src, num);
        }
        // finalizes the digest
        void finalize(unsigned char * dst, uint32 * len) {
            EVP_DigestFinal_ex(&md, dst, len);
        }

        // reinitalizes the digest
        void reinit() {
            EVP_DigestInit(&md, m_evp);
        }

        // returns the digest size
        uint32 getDigestSize() {
            EVP_MD_size(m_evp);
        }

        void hash(const byte * src, int srclen,byte * digest,uint32 * dlen)
        {
            reinit();
            update(src, srclen);
            finalize(digest, dlen);
        }

    protected:
        EVP_MD_CTX md;
        const EVP_MD * m_evp;
    };
};

#endif