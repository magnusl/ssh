#ifndef _ALGORITHMS_H_
#define _ALGORITHMS_H_

#include "cipher_base.h"
#include "hmac_base.h"
#include "common.h"

namespace ssh
{
    /* Struct:          Algorithms
     * Description:     Wrapper for the used algorithms.
     */
    struct Algorithms
    {
    public:
        // initalizes the elements correctly.
        Algorithms() : cipher(0), hmac(0) {
        }

        // destructor, performs the required cleanup
        ~Algorithms() 
        {
            SAFE_DELETE(cipher)
            SAFE_DELETE(hmac)
        }
        cipher_base * cipher;       // the used cipher
        hmac_base   * hmac;         // the used mac algorithm
    };
};
#endif