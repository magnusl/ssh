#ifndef _SHA1_H_
#define _SHA1_H_

#include "HashBase.h"

namespace ssh
{

    /* Class:           sha1
     * Description:     SHA1 hash
     */
    class sha1 : public HashBase
    {
    public:
        sha1() : HashBase(EVP_sha1()) {
        }
    };
};

#endif