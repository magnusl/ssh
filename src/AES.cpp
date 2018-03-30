/* File:            AES.cpp
 * Description:     Implements the AES cipher in CBC mode.
 * Author:          Magnus Leksell
 *
 * Copyright © 2006-2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/

#include "AES.h"
#include <cstdlib>
#include <memory>

namespace ssh
{
    /* Function:        AES::decryptInit
     *
     */
    bool AES::decryptInit(const unsigned char * userKey, const unsigned char * iv)
    {
        // initalize the decryption key.
        AES_set_decrypt_key(userKey, (key_length * 8), &key);
        // copy the IV
        memcpy(IV, iv, AES_BLOCK_SIZE);
        return true;
    }

    /* Function:        AES::encryptInit
     *
     */
    bool AES::encryptInit(const unsigned char * userKey, const unsigned char * iv)
    {
        // initalize the decryption key.
        AES_set_encrypt_key(userKey, (key_length * 8), &key);
        // copy the IV
        memcpy(IV, iv, AES_BLOCK_SIZE);
        return true;
    }

    /* Function:        AES::encrypt
     * Description:     Encrypts the data with the AES cipher in CBC mode
     */
    bool AES::encrypt(const unsigned char * in, unsigned char * out, int size)
    {
        AES_cbc_encrypt(in, out,size, &key,IV, AES_ENCRYPT);
        return true;
    }

    /* Function:        AES::decrypt
     * Description:     Decrypts the data with the AES cipher in CBC mode
     */
    bool AES::decrypt(const unsigned char * in, unsigned char * out, int size)
    {
        AES_cbc_encrypt(in, out,size, &key,IV, AES_DECRYPT);
        return true;
    }
};