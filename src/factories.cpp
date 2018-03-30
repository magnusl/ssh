/* File:            factories.cpp
 * Description:     Contains the implementation of the factory functions used 
 *                  to create the algorithm instances under runtime.
 * Author:          Magnus Leksell
 *
 * Copyright © 2006-2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/

#include <iostream>

#include "cipher_base.h"
#include "hmac_base.h"
#include "kex_base.h"
#include "hostkey_base.h"
#include "DH_groups.h"
#include "DH_keyexchange.h"
#include "AES.h"
#include "hmac-sha1.h"
#include "dss.h"
#include "RSA.h"

namespace ssh
{

    using namespace std;

    /* Function:        create_cipher
     * Description:     Creates the cipher instance to be used.
     */
    cipher_base * create_cipher(const std::string & name)
    {
        cipher_base * instance = 0;
        if(name == "aes128-cbc") {
            instance = new (std::nothrow) AES(16);
        } else if(name == "aes192-cbc") {
            instance = new (std::nothrow) AES(24);
        } else if(name == "aes256-cbc") {
            instance = new (std::nothrow) AES(32);
        }
        return instance;
    }

    /* Function:        create_hmac 
     * Description:     Creates the requested Keyed Message Authentication Code.
     */
    hmac_base * create_hmac(const std::string & name)
    {
        hmac_base * instance = 0;
        if(name == "hmac-sha1")
            instance = new (std::nothrow) hmac_sha1();
        return instance;
    }

    /* Function:        create_kex
     * Description:     Creates the keyexchange algorithm instance.
     */
    kex_base * create_kex(const std::string & name)
    {
        kex_base * instance = NULL;
        if(name == "diffie-hellman-group1-sha1") {
            instance = new (std::nothrow) DH_keyexchange(DH_group1_safe_prime,DH_group1_generator);
            if(!instance) {
                cerr << "Failed to allocate memory for the kex instance" << endl;
                return NULL;
            }
        } else if(name == "diffie-hellman-group14-sha1")
        {
            instance = new (std::nothrow) DH_keyexchange(DH_group14_safe_prime,DH_group14_generator);
            if(!instance) {
                cerr << "Failed to allocate memory for the kex instance" << endl;
                return NULL;
            }
        }
        return instance;
    }

    /* Function:        create_hostkey
     * Description:     Creates the instance for the specified hostkey algorithm.
     */
    hostkey_base * create_hostkey(const std::string & name)
    {
        if(name == "ssh-dss") 
            return new (std::nothrow) DSS();
        else if(name == "ssh-rsa") 
            return new (std::nothrow) RSA_hostkey();
        else 
            return 0;
    }
};