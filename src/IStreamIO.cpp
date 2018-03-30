/* File:            IStreamIO
 * Description:     Contains the functions for writing and reading from a 
 *                  stream.
 * Author:          Magnus Leksell
 *
 * Copyright © 2006-2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/

#include "IStreamIO.h"
#include "SocketLayer.h"
#include "definitions.h"
#include "common.h"
#include <assert.h>
#include <iostream>

using namespace std;

namespace ssh
{
    /* Function:        IStreamIO::IStreamIO 
     * Description:     Constructor.
     */
    IStreamIO::IStreamIO()
    {
        // default to ASNI
#ifdef _WIN32
        m_nEncoding = CP_ACP;
#endif
    }

    /* Read functions */

    /* Function:        IStreamIO::readBigInteger
     * Description:     Reads a large integer from the stream.
     */
    bool IStreamIO::readBigInteger(BIGNUM ** num)
    {
        unsigned char number[MAX_NUMBER_LENGTH];
        uint32 length = MAX_NUMBER_LENGTH;
        if(num == NULL) return false;

        if(!readString(number, length))
            return false;
        if(length > MAX_NUMBER_LENGTH) return false;
        *num = BN_bin2bn(number, length, 0);
        if(!(*num)) {
            std::cerr << "IStreamIO::readBigInteger: Could not the read string to a large integer" << std::endl;
            return false;
        }
        if((*num)->neg) {
            std::cerr << "IStreamIO::readBigInteger: Only positive numbers is supported" << std::endl;
            BN_free(*num); *num = 0;
            return false;
        }
        return true;
    }

    /* Function:        IStreamIO::readByte
     * Description:     Reads a single byte from the stream
     */
    bool IStreamIO::readByte(unsigned char & byte)
    {
        unsigned char b;
        if(!readBytes(&b, sizeof(char)))
            return false;
        byte = b;
        return true;
    }

    /* Function:        IStreamIO::readInt
     * Description:     Reads a uint32eger from the stream
     */
    bool IStreamIO::readInt32(uint32 & num)
    {
        uint32 num_be;
        if(!readBytes(reinterpret_cast<unsigned char *>(&num_be), sizeof(uint32)))
            return false;
        num = __ntohl32(num_be);
        return true;
    }

    /* Function:        IStreamIO::readInt
     * Description:     Reads a unsigned 16 bit integer from the stream.
     */
    bool IStreamIO::readInt16(uint16 & num)
    {
        uint16 num_be;
        if(!readBytes(reinterpret_cast<unsigned char *>(&num_be), sizeof(uint16)))
            return false;
        num = __ntohl16(num_be);
        return true;
    }

    /* Function:        IStreamIO::readInt
     * Description:     Reads a unsigned 64 bit integer from the stream.
     */
    bool IStreamIO::readInt64(uint64 & num)
    {
        uint64 num_be;
        if(!readBytes(reinterpret_cast<unsigned char *>(&num_be), sizeof(uint64)))
            return false;
        num = __ntohl64(num_be);
        return true;

    }
     
    /* Function:        IStreamIO::readString
     * Description:     Reads a ascii string from the buffer.
     */
    bool IStreamIO::readString(std::string & str)
    {
        char buff[MAX_STRING_LENGTH + 1];
        uint32 len;
        if(!readInt32(len))
            return false;
        if(len == 0) return true;
        if(len > MAX_STRING_LENGTH) return false;
        if(!readBytes((unsigned char *)buff, len))
            return false;
        // null terminate the array
        buff[len] = '\0';
        str = buff;
        return true;
    }

    /* Function:        IStreamIO::readString
     * Description:     Reads a binary string. The 'len' parameter should be total size 
     *                  the buffer. The 'len' variable will be updated with the length read.
     */
    bool IStreamIO::readString(unsigned char * buffer, uint32 & len)
    {
        if(len == 0) return false;
        uint32 readlen;
        if(!readInt32(readlen))
            return false;
        if(readlen > len) 
            return false;
        if(!readBytes(buffer, readlen))
            return false;
        len = readlen;
        return true;
    }

    /*
     * Write functions.
     */

    /* Function:        IStreamIO::writeBigInteger
     * Description:     Writes a OpenSSL BIGNUM to the stream.
     */
    bool IStreamIO::writeBigInteger(BIGNUM * bn)
    {
        if(BN_is_zero(bn)) {
            return writeInt32(static_cast<uint32>(0));
        }
        if(BN_is_negative(bn)) {
            return false;
        }
        int numBytes = BN_num_bytes(bn) + 1;
        if(numBytes < 2) {
            return false;
        }
        // get the length required to store the data
        unsigned char * data = new (std::nothrow) unsigned char[numBytes];
        if(data == NULL) {
            cerr << "Could not allocate memory" << endl;
            return false;
        }

        // padding byte
        data[0] = 0x00;
        // convert the BN to the right representation
        int val = BN_bn2bin(bn, data+1);
        if(val != numBytes - 1) {
            SAFE_DELETE_ARRAY(data)
            return false;
        }
        // check the first byte
        int nohighbit = (data[1] & 0x80) ? 0 : 1;
        uint32 len = (uint32)(numBytes - nohighbit);
        writeInt32(len);
        writeBytes(data + nohighbit,len);
        SAFE_DELETE_ARRAY(data)
        return true;
    }

    /* Function:        IStreamIO::writeByte
     * Description:     Writes a single byte to the stream.
     */
    bool IStreamIO::writeByte(unsigned char b)
    {
        return writeBytes(&b, sizeof(char));
    }

    /* Function:        writeInt
     * Description:     Write a 16 bits uint32eger to the stream.
     */
    bool IStreamIO::writeInt16(uint16 num)
    {
        num = htons(num);
        return writeBytes(reinterpret_cast<unsigned char *>(&num), sizeof(uint16));
    }

    /* Function:        IStreamIO::writeInt
     * Description:     Writes a 32 bit uint32eger to the stream.
     */
    bool IStreamIO::writeInt32(uint32 num)
    {
        num = __htonl32(num);
        return writeBytes(reinterpret_cast<unsigned char *>(&num) , sizeof(uint32));
    }

    /* Function:        IStreamIO::writeInt
     * Description:     Writes a 64 bit uint32eger to the stream.
     */
    bool IStreamIO::writeInt64(uint64 num)
    {
        num = __htonl64(num);
        return writeBytes(reinterpret_cast<unsigned char *>(&num), sizeof(uint64));
    }

    /* Function:        IStreamIO::writeString
     * Description:     Writes a ascii string to the stream.
     */
    bool IStreamIO::writeString(const std::string & str)
    {
        uint32 len = static_cast<uint32>(str.length());
        return (writeInt32(len)&& writeBytes(reinterpret_cast<const unsigned char *>(str.c_str()), len));
    }

    /* Function:        IStreamIO::writeString
     * Description:     Writes a string to the stream.
     */
    bool IStreamIO::writeString(const unsigned char * str, uint32 count)
    {
        return (writeInt32(count) && writeBytes(str, count));
    }

    /* Function:        IStreamIO::writeString
     * Description:     Writes a normal c-string (char *) to the stream.
     */
    bool IStreamIO::writeString(const char * str)
    {
        uint32 len = (uint32) strlen(str);
        return (writeInt32(len) && writeBytes(reinterpret_cast<const unsigned char *>(str), len));
    }

    /* Function:        IStreamIO::readString
     * Description:     Allocates the required memory for the string and reads it from the stream.
     */
    bool IStreamIO::readString(unsigned char ** buff, uint32 & len)
    {
        if(!readInt32(len))
            return false;
        if(len > MAX_STRING_LENGTH)
            return false;
        if(len == 0) {
            *buff = 0;
            return true;
        }
        *buff = new (std::nothrow) unsigned char[len];
        if((*buff) == NULL)
            return false;
        if(!readBytes(*buff, len)) {
            delete [] *buff;
            *buff = NULL;
            return false;
        }
        return true;
    }

    /* File:            IStreamIO::readFile
     * Description:     Geneeric "slow" implementation,should be overriden by ArrayStream etc.
     */
    bool IStreamIO::readFile(FILE * file, int num)
    {
        if(file == NULL || num < 0) return false;
        if(num == 0) return true;

        uint32 to_read = 0, left = num;
        unsigned char buff[1024];
        while(left > 0) {
            to_read = (left > 1024 ? 1024 : left);
            if(!readBytes(buff, to_read)) {
                cerr << "Could not read the requested number of bytes from the stream" << endl;
                return false;
            }
            // now write the data to the file
            if(fwrite(buff, to_read, 1, file) != 1) {
                cerr << "Could not wrtite the requested data to the file" << endl;
                return false;
            }
            left -= to_read;
        }
        return true;
    }

    /* Function:        IStreamIO::writeFile
     * Description:     Write contents from the file to the stream. A generic slow implementation.
     *                  ArrayStream etc has faster implementation.
     */
    bool IStreamIO::writeFile(FILE * file, int num)
    {
        if(file == NULL || num < 0) return false;
        if(num == 0) return true;
        uint32 to_write = 0, left = num;
        unsigned char buff[4096];
        while(left > 0) {
            to_write = (left > 4096 ? 4096 : left);
            if(fread(buff, to_write, 1, file) != 1) {
                cerr << "could not read the requested data from the file" << endl;
                return false;
            }
            // now write the data to the stream
            if(!writeBytes(buff, to_write)) {
                cerr << "Could not write the requested data to the stream" << endl;
                return false;
            }

            left -= to_write;
        }
        return true;

    }
        

#ifdef _WIN32
    /* Function:        IStreamIO::readWideString
     * Description:     Reads a string from stream from the buffer and converts it using the correct
     *                  encoding method to a wide character string. Win32 only
     */
    bool IStreamIO::readWideString(std::wstring & str_wide, int enc)
    {
        /*
         * Set the encoding
         */
        uint32 encoding;
        if(enc == 0) encoding = m_nEncoding;
        else {
            if(enc == SSH_ENCODING_UTF_8) encoding = CP_UTF8;
            else if(enc == SSH_ENCODING_ANSI) encoding = CP_ACP;
            else return false;
        }
        uint32 len;
        if(!readInt32(len)) {
            std::cerr << "Could not read the length of the string" << std::endl;
            return false;
        }
        if(len > MAX_STRING_LENGTH) {
            std::cerr << "String to long" << std::endl;
            return false;
        }
        if(len == 0) {
            // a empty string
            return true;
        }
        /* read the binary data */
        char str[MAX_STRING_LENGTH];
        if(!readBytes((unsigned char *)str, len)) {
            std::cerr << "Could not read the bytes of the string" << std::endl;
            return false;
        }

        /* calculate how much space is required */
        int wideLength = MultiByteToWideChar(encoding,/*MB_ERR_INVALID_CHARS */ 0, str, len, 0,0);
        if(wideLength == 0) {
            std::cerr << "Could not calculate the length of the wide char string" << std::endl;
            return false;
        }
        wchar_t * widestr = new (std::nothrow) wchar_t[wideLength + 1];
        if(!widestr) 
            return false;
        memset(widestr, 0, sizeof(wchar_t) * (wideLength + 1));
        /* convert the string */
        if(!MultiByteToWideChar(encoding, /*MB_ERR_INVALID_CHARS*/ 0, str, len,  widestr, wideLength + 1)) {
            std::cerr << "Could not convert the string" << std::endl;
            delete [] widestr;
            return false;
        }
        str_wide = widestr;
        delete [] widestr;
        return true;
    }

    bool IStreamIO::writeWideString(const wchar_t * str, int enc)
    {
        std::wstring wstr = str;
        return writeWideString(wstr, enc);
    }
    /*
     *
     */
    bool IStreamIO::writeWideString(const std::wstring & str, int enc)
    {
        /*
         * Set the encoding
         */
        uint32 encoding;
        if(enc == 0) {
            encoding = m_nEncoding;
        } else {
            if(enc == SSH_ENCODING_UTF_8) encoding = CP_UTF8;
            else if(enc == SSH_ENCODING_ANSI) encoding = CP_ACP;
            else {
                return false;
            }
        }

        if(str.length() == 0) {
            return writeInt32(0);
        }

        char * strMultiByte = 0;
        int length;

        // first check how many bytes are required.
        length = WideCharToMultiByte(encoding, 0 , str.c_str(), -1, 0, 0, 0, 0);
        if(length == 0 || length > MAX_STRING_LENGTH)
            return false;

        // write the length in bytes,
        if(!writeInt32(static_cast<uint32>(length - 1)))    // -1 to skip the null char.
            return false;

        // now allocate memory for the string
        strMultiByte = new (std::nothrow) char[length];
        if(strMultiByte == NULL) {
            cerr << "Could not allocate the required memory for the multibyte string" << endl;
            return false;
        }
        // now perform the conversion
        if(!WideCharToMultiByte(encoding,0, str.c_str(), -1, strMultiByte, length, 0, 0))
        {
            // the conversion failed.
            delete [] strMultiByte;
            return false;
        }
        // write the string to the buffer
        if(!writeBytes(reinterpret_cast<const unsigned char *>(strMultiByte), length - 1))
        {
            // failed to write the stream
            delete [] strMultiByte;
            return false;
        }

        // String has been written.
        delete [] strMultiByte;
        return true;
    }

    /* Function:        IStreamIO::SetEncoding
     * Description:     Sets the encoding to be used.
     */
    bool IStreamIO::SetEncoding(uint32 enc)
    {
        if(enc == SSH_ENCODING_UTF_8) {
            // Use UTF-8
            m_nEncoding = CP_UTF8;
            return true;
        }
        else if(enc == SSH_ENCODING_ANSI) {
            // Use ANSI
            m_nEncoding = CP_ACP;
            return false;
        }
        else {
            // Invalid encoding.
            return false;
        }
    }
#endif
};