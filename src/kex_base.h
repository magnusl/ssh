#ifndef _KEX_BASE_H_
#define _KEX_BASE_H_

#include "exchange_data.h"
#include "types.h"
#include "ssh_context.h"

namespace ssh
{
    class ssh_transport;

    /* Class:           kex_base
     * Description:     Baseclass for the keyexchange algorithms.
     */
    class kex_base
    {
    public:
        virtual int perform_keyexchange(ssh_transport *, exchange_data *) = 0;
        virtual int calculate_exchangehash(const ssh_context &, const ssh_context &, exchange_data *) = 0;
        virtual const char * hash() const = 0;
        virtual uint32 get_digest_length() = 0;
        static const char * DEFAULT_KEXS;

    };
};

#endif