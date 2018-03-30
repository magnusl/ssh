/* File:            dss.cpp
 * Description:     Handles host-verification using DSA
 * Author:          Magnus Leksell
 * Copyright © 2006-2008 Magnus Leksell, all rights reserved.
 *****************************************************************************/

#include "dss.h"
#include <openssl/dsa.h>
#include <openssl/engine.h>
#include "ArrayStream.h"
#include <iostream>
#include "sha1.h"
#include <openssl/err.h>
#include "HashStream.h"
#include "md5.h"
#include "sha1.h"

using namespace std;

namespace ssh
{

    /* Function:        DSS::DSS()
     * Description:     Performs the required initalization
     */
    DSS::DSS()
    {
        dsa = DSA_new();
        sig = NULL;
    }

    /* Function:        DSS::~DSS()
     * Description:     Performs the required cleanup.
     */
    DSS::~DSS()
    {
        if(dsa) DSA_free(dsa);
        if(sig) DSA_SIG_free(sig);
    }

    /* Function:        DSS::parse_keyblob
     * Description:     Parses the DSS keyblob and extracts the servers public key.
     */
    bool DSS::parse_keyblob(unsigned char * keyblob, uint32 len)
    {
        if(!dsa) 
            return false;
        ssh::ArrayStream as(keyblob, len);
        // read the identifier
        std::string ident;
        if(!as.readString(ident)) {
            cerr << "DSS::parse_keyblob: could not read the identifier from the keyblob" << endl;
            return false;
        }
        
        if(!(ident == "ssh-dss")) {
            cerr << "DSS::parse_keyblob: not a dss key" << endl;
            return false;
        }

        // now read the public key.
        if(!as.readBigInteger(&dsa->p) ||
            !as.readBigInteger(&dsa->q) ||
            !as.readBigInteger(&dsa->g) ||
            !as.readBigInteger(&dsa->pub_key))
        {
            cerr << "DSS::parse_keyblob: could not read the keys from the keyblob" << endl;
            return false;
        }

        /* Hash the keyblob to get the fingerprint */
        return true;
    }

    /* Function:        DSS::parse_signature
     * Description:     Parses the supplied signature.
     */
    bool DSS::parse_signature(unsigned char * signblob, uint32 len)
    {
        ssh::ArrayStream as(signblob, len);     
        std::string ident;
        if(!as.readString(ident)) {
            cerr << "DSS::parse_signature: could not read the identifier from the signature" << endl;
            return false;
        }
        // check the signature type.
        if(!(ident == "ssh-dss")) {
            cerr << "DSS::parse_signature: not a dss signature blob" << endl;
            return false;
        }
        // the actual signature is encoded as a string. 
        uint32 siglen;
        if(!as.readInt32(siglen)) {
            cerr << "DSS::parse_signature: Failed to read the length field" << endl;
            return false;
        }
        if(siglen != (DSS_R_LENGTH + DSS_S_LENGTH)) {
            cerr << "DSS::parse_signature: Invalid signature length" << endl;
            return false;
        }
        unsigned char r[DSS_R_LENGTH], s[DSS_S_LENGTH];
        if(!as.readBytes(r,DSS_R_LENGTH) || !as.readBytes(s, DSS_S_LENGTH))
        {
            cerr << "DSS::parse_signature: could not read the signature from the signatureblob" << endl;
            return false;

        }

        if(!(sig = DSA_SIG_new())) {
            cerr << "DSS::parse_signature: DSA_SIG_new() failed" << endl;
            return false;
        }

        sig->r = BN_new();
        sig->s = BN_new();

        if(!sig->r || !sig->s) {
            cerr << "DSS::parse_signature: allocation failed" << endl;
            return false;
        }

        BN_bin2bn(r, DSS_R_LENGTH, sig->r);
        BN_bin2bn(s, DSS_S_LENGTH, sig->s);
        return true;
    }

    /* Function:        DSS::verify_host
     * Description:     Verifies the host.
     */
    int DSS::verify_host(unsigned char * H, uint32 len)
    {
        if(!dsa) return false;
        ERR_load_crypto_strings();
        unsigned char digest[MAX_DIGEST_LENGTH];
        uint32 dlen = MAX_DIGEST_LENGTH;
        
        // hash it using the sha1 algorithm
        sha1().hash(H, len, digest, &dlen);
        // Create the signature object
        int nret = DSA_do_verify(digest, dlen, sig, dsa);
        if(nret == 1) {
            return VALID_SIGNATURE;
        } else if(nret == 0) {
            std::cerr << "Invalid signature"<< endl;
            return INVALID_SIGNATURE;
        } else {
            std::cerr << "Verification failed: " <<  ERR_error_string(ERR_get_error(), NULL) << std::endl;
            return STATUS_FAILURE;
        }
    }

    /*
     * Writes the keyblob to the stream. This is used when calculating the fingerprint.
     */
    bool DSS::hash_keyblob(const char * hash, byte * digest, uint32 * dlen) const
    {
        HashStream stream(hash);
        if(!stream) 
            return false;
        stream.writeBigInteger(dsa->p);
        stream.writeBigInteger(dsa->q);
        stream.writeBigInteger(dsa->g);
        stream.writeBigInteger(dsa->pub_key);
        stream.digest(digest, dlen);
        return true;
    }

    /*
     * Returns the length of the public key, in bits.
     */
    size_t DSS::numbits() const
    {
        return (size_t) BN_num_bits(dsa->pub_key);
    }
};
