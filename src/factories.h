#ifndef _FACTORIES_H_
#define _FACTORIES_H_

namespace ssh
{
    cipher_base * create_cipher(const std::string & name);
    hmac_base * create_hmac(const std::string & name);
    kex_base * create_kex(const std::string & name);
    hostkey_base * create_hostkey(const std::string & name);
};

#endif