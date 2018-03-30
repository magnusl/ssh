#ifndef _MD5_H_
#define _MD5_H_

#include "HashBase.h"

namespace ssh
{

    /* Class:           md5
     * Description:     MD5 hash
     */
    class md5 : public HashBase
    {
    public:
        md5() : HashBase(EVP_md5()) {
        }
    };
};

#endif