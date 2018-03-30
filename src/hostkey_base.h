/* File:            hostkey_base.h
 * Description:     A hostkey algorithm is used to provide host authentication.
 * Author:          Magnus Leksell
 * 
 * Copyright © 2006-2007 Magnus Leksell, all rights reserved.
 ******************************************************************************/

#ifndef _HOSTKEY_BASE_H_
#define _HOSTKEY_BASE_H_

#include "types.h"
//#include "key_formats.h"
//#include "signature_formats.h"
#include "definitions.h"
#include "IStreamIO.h"
#include "HashBase.h"

namespace ssh
{

    /* Class:           hostkey_base
     * Description:     Base class for the hostkey algorithms. Used to provide host 
     *                  authentication.
     */
    class hostkey_base
    {
    public:
        // parses the keyblob.  
        virtual bool parse_keyblob(unsigned char *, uint32)                         = 0;    
        // parses the keyblob.  
        virtual bool parse_signature(unsigned char *, uint32)                       = 0;
        // verifies the host    
        virtual int verify_host(unsigned char *, uint32)                            = 0;    
        // retrives the keyblob.
        virtual bool hash_keyblob(const char *, byte *, uint32 *) const             = 0;
        // returns the identifier string
        virtual const char * identifier() const = 0;
        virtual size_t numbits() const = 0;
        // creates a finger print of the key
        bool fingerprint(const char *,std::string &) const;
    };
};
#endif