/*
 *
 *
 *
 *
 ************************************************************************'****/

#ifndef _DSS_H_
#define _DSS_H_

#include "hostkey_base.h"
#include <openssl/bn.h>
#include <openssl/dsa.h>

#define DSS_R_LENGTH    20
#define DSS_S_LENGTH    20

namespace ssh
{

    /* Class:           DSS     
     * Description:     The Digital Signature Standard.
     */
    class DSS : public hostkey_base
    {
    public:
        DSS();
        ~DSS();
        bool parse_keyblob(unsigned char *, uint32);    // parses the keyblob.  
        int verify_host(unsigned char *, uint32);   // verifies the host authenticy
        bool parse_signature(unsigned char *, uint32);
        bool hash_keyblob(const char *, byte *, uint32 *) const;
        const char * identifier() const {return "dsa";}
        size_t numbits() const;
    private:
        DSA * dsa;
        DSA_SIG * sig;
    };
};


#endif