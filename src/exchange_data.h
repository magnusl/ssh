/* File:            exchange_data.h
 * Description:     Defines a container class that stores the required 
 *                  information to perform the keyexchange.
 * Author:          Magnus Leksell
 * 
 * Copyright 2006-2007 Magnus Leksell, all rights reserved
 *****************************************************************************/

#ifndef _EXCHANGE_DATA_H_
#define _EXCHANGE_DATA_H_

#include <openssl/bn.h>
#include "common.h"
#include "ssh_hdr.h"

namespace ssh
{

    /* class:           exchange_data
     * Description:     Contains the required information to perform the keyexchange and
     *                  derive the required keys. Will performed the required cleanup when
     *                  the object is destroyed.
     */
    class exchange_data
    {
    public:

        exchange_data() 
        {
            // NULL the pointers.
            keyblob         = NULL;
            signature       = NULL;
            shared_secret   = NULL;
            exchange_hash   = NULL;
            server_pubkey   = NULL;
        }

        ~exchange_data()
        {
            // delete the data.
            SAFE_DELETE_ARRAY(keyblob)
            SAFE_DELETE_ARRAY(signature)
            SAFE_DELETE_ARRAY(exchange_hash)
            if(shared_secret) BN_free(shared_secret);
            if(server_pubkey) BN_free(server_pubkey);
        }

        unsigned char * keyblob, * signature, * exchange_hash;      // the keyblob,signature and the exchange hash.
        uint32 keyblob_len, signature_len, exchange_len;        // the lengths of the keyblob and signature
        BIGNUM * shared_secret;                                     // the shared secret
        BIGNUM * server_pubkey;                                     // the servers public key.
        ssh_hdr client_kex_hdr, server_kex_hdr;                     // The headers.
    };
};

#endif