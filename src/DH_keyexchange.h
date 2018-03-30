#ifndef _DH_KEYEXCHANGE_H_
#define _DH_KEYEXCHANGE_H_

#include "kex_base.h"
#include <openssl/dh.h>
#include <openssl/engine.h>

namespace ssh
{
    /* Class:           DH_keyexchange
     * Description:     Diffie-Hellman keyexchange.
     */
    class DH_keyexchange : public kex_base
    {
    public:
        DH_keyexchange(const char * prime, const char * generator) 
        {
            m_p = prime;
            m_g = generator;
        }
        int perform_keyexchange(ssh_transport *, exchange_data *);          // performs the keyexchange.
        int calculate_exchangehash(const ssh_context &, const ssh_context &, exchange_data *);
        uint32 get_digest_length() {return 20;}
        const char * hash() const {return "sha1";}
    protected:
        bool generate_keys();
        int parse_kexdh_reply(ssh_transport *, exchange_data *);
        int calculate_shared_secret(exchange_data *);
        int calculate_exchangehash();

        const char  * m_p,              // the safe prime 
                    * m_g;              // the generator.

        DH          * m_dh;             // Diffie Hellman struct.

    };
};


#endif