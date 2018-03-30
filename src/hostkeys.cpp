#include "hostkey_base.h"
#include "ArrayStream.h"
#include "hex.h"
#include <iostream>
#include <sstream>
#include "HashStream.h"

namespace ssh
{
    using namespace std;
    /* Function:        hostkey_base::fingerprint
     * Description:     Hashes the key and creates the fingerprint using the supplied hash.
     */
    bool hostkey_base::fingerprint(const char * hash, std::string & fingerprint) const
    {
        byte digest[EVP_MAX_MD_SIZE];
        uint32 dlen;
        string hexblob;
        stringstream ss;
        // hash the keyblob.
        if(!hash_keyblob(hash, digest, &dlen))
            return false;
        // now convert the binary data to a hexadecimal string.
        ssh_bin2hex(digest, dlen, hexblob,true);
        // now create the actual fingerprint
        ss << identifier() << " " << numbits() << " " << hexblob;
        fingerprint = ss.str();
        return true;
    }
};