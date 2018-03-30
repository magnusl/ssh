#ifndef _CIPHER_BASE_H_
#define _CIPHER_BASE_H_

#include "types.h"

namespace ssh
{
    /* Class:           cipher_base
     * Description:     Baseclass for the ciphers.
     */
    class cipher_base
    {
    public:
        // default constructor
        cipher_base(uint32 keylength) : key_length(keylength) {}
        // destructor
        virtual ~cipher_base() {}
        // returns the key size
        virtual uint32 getKeySize() {return key_length;}
        // initalizes decryption
        virtual bool decryptInit(const unsigned char *, const unsigned char *) = 0;
        // initalizes encryption
        virtual bool encryptInit(const unsigned char *, const unsigned char *) = 0;
        // encrypts data
        virtual bool encrypt(const unsigned char * in, unsigned char * out, int size) = 0;
        // decrypts data.
        virtual bool decrypt(const unsigned char * in, unsigned char * out, int size) = 0;
        virtual uint32 getBlockSize() = 0;

        static const char * DEFAULT_CIPHERS;
    protected:
        uint32 key_length;
    };
};

#endif