/* File:            rsa.cpp
 * Description:     The RSA hostkey algorithm is used to authenticate a host.
 * Author:          Magnus Leksell
 *
 * Copyright © 2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/

#include "rsa.h"
#include "assert.h"
#include "ArrayStream.h"
#include <iostream>
#include "sha1.h"
#include "HashStream.h"
#include "md5.h"
#include "sha1.h"

using namespace std;

namespace ssh
{

    /* Function:        RSA_hostkey::RSA_hostkey()
     * Description:     Performs the required initalization.
     */
    RSA_hostkey::RSA_hostkey()
    {
        rsa = RSA_new();
        signature = 0;
    }

    /* Function:        RSA_hostkey::~RSA_hostkey()
     * Description:     Performs the required cleanup.
     */
    RSA_hostkey::~RSA_hostkey()
    {
        if(rsa) RSA_free(rsa);
        delete [] signature;
    }

    /* Function:        RSA::parse_keyblob
     * Description:     Parses the RSA keyblob.
     */
    bool RSA_hostkey::parse_keyblob(unsigned char * keyblob, uint32 len)
    {
        if(!rsa) return false;
        string ident;
        ssh::ArrayStream as(keyblob, len);
        if(!as.readString(ident)) {
            return false;
        }
        // read the identifier
        if(ident != "ssh-rsa") {
            cerr << "Not a RSA keyblob" << endl;
            return false;
        }
        // read the public key
        if(!as.readBigInteger(&rsa->e) ||  !as.readBigInteger(&rsa->n)) {
            cerr << "Could not read the remote hosts public RSA key from the keyblob" << endl;
            return false;
        }
        return true;
    }

    /* Function:        RSA_hostkey::parse_signature
     * Description:     Parses the RSA signature blob.
     */
    bool RSA_hostkey::parse_signature(unsigned char * sign, uint32 len)
    {
        ssh::ArrayStream as(sign, len);
        if(rsa == NULL) return false;

        string ident;
        if(!as.readString(ident)) {
            cerr << "could not read the ident from the signature blob" << endl;
            return false;
        }

        if(ident != "ssh-rsa") {
            cerr << "Not a RSA signature" << endl;
            return false;
        }

        // read the signature
        if(!as.readInt32(sign_len)) {
            cerr << "could not read the signature length from the stream" << endl;
            return false;
        }

        // Some sanity checks.
        if(sign_len == 0) {
            cerr << "Empty signature, signature length = 0" << endl;
            return false;
        }

        if(sign_len > MAX_SIGNATURE_LENGTH) {
            cerr << "The signature is to large " << endl;
            return false;
        }

        // allocate the required memory
        signature = new (std::nothrow) unsigned char[sign_len];
        if(signature == NULL) {
            cerr << "Could not allocate the required memory for the signature" << endl;
            return false;
        }

        // read the actual signature
        if(!as.readBytes(signature, sign_len)) {
            cerr << "Could not read the signature from the stream" << endl;
            return false;
        }
        return true;
    }

    /* Function:        RSA_hostkey::verify_host
     * Description:     Performs the host verification.
     */
    int RSA_hostkey::verify_host(unsigned char * H, uint32 len)
    {
        ERR_load_crypto_strings();
        unsigned char digest[MAX_DIGEST_LENGTH];
        uint32 dlen = MAX_DIGEST_LENGTH;

        // hash it
        sha1().hash(H,len,digest,&dlen);
        if(!rsa) return STATUS_FAILURE;
        int nret = RSA_verify(NID_sha1, digest, dlen, signature, sign_len, rsa);
        if(nret == 0) {
            return INVALID_SIGNATURE;
        } else if(nret == 1) {
            return VALID_SIGNATURE;
        } else {
            std::cerr << "ERR_error_string(ERR_get_error() , NULL)" << std::endl;
            return STATUS_FAILURE;
        }
    }


    bool RSA_hostkey::hash_keyblob(const char * hash, byte * digest, uint32 * dlen) const
    {
        HashStream stream(hash);
        if(!stream) 
            return false;
        stream.writeString("ssh-rsa");
        stream.writeBigInteger(rsa->e);
        stream.writeBigInteger(rsa->n);
        stream.digest(digest, dlen);
        return true;
    }

    /*
     * Returns the length of the public key, in bits.
     */
    size_t RSA_hostkey::numbits() const
    {
        return (size_t) BN_num_bits(rsa->n);
    }
};