/* File:            AES.h
 * Description:     Contains the declaration of the Advanced Encryption
 *                  Standard (AES) in Cipher Block Chaining mode. 
 *                  Uses the OpenSSL implementation of AES.
 * Author:          Magnus Leksell
 *
 * Copyright © 2006-2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/

#ifndef _AES_H_
#define _AES_H_

#include "cipher_base.h"
#include "types.h"
#include <openssl/aes.h>

namespace ssh
{

    /* Class:           AES
     * Description:     The Advanced Encryption Standard (AES) cipher in CBC mode.
     */
    class AES : public cipher_base 
    {
    public:
        AES(uint32 _keysize) : cipher_base(_keysize) {
        }
        // initalizes decryption
        bool decryptInit(const unsigned char *, const unsigned char *);
        // initalizes encryption
        bool encryptInit(const unsigned char *, const unsigned char *);
        // encrypts data
        bool encrypt(const unsigned char * in, unsigned char * out, int size);
        // decrypts data.
        bool decrypt(const unsigned char * in, unsigned char * out, int size);
        // returns the blocksize of the AES algorithm.
        uint32 getBlockSize() {return AES_BLOCK_SIZE;}

    protected:
        unsigned char IV[AES_BLOCK_SIZE]; // 16 bytes IV
        AES_KEY key;
    };
};
#endif