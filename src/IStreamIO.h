#ifndef _ISTREAMIO_H_
#define _ISTREAMIO_H_

#include "types.h"
#include <openssl/bn.h>
#include <string>
#include "BoundedBuffer.h"
#include <cstdio>
#include "definitions.h"

namespace ssh
{
    /* Class:           IStreamIO
     * Description:     Interface/Baseclass for classes that should implement
     *                  a stream I/O type of operation. 
     */
    class IStreamIO
    {
    public:

        IStreamIO();

        virtual bool writeBytes(const unsigned char *, uint32) = 0;
        virtual bool readBytes(unsigned char *, uint32) = 0;

        /* write functions */
        bool writeByte(unsigned char);
        bool writeInt16(uint16);
        bool writeInt32(uint32);
        bool writeInt64(uint64);
        bool writeBigInteger(BIGNUM *);
        bool writeString(const std::string &);
        bool writeWideString(const std::wstring &, int enc = 0);
        bool writeWideString(const wchar_t *, int enc = 0);
        bool writeString(const unsigned char *, uint32);
        bool writeString(const char *);
        virtual bool writeFile(FILE *, int);        // reads data from the file and writes it to the stream.

        /* read functions */
        bool readByte(unsigned char &);
        bool readInt16(uint16 &);
        bool readInt32(uint32 &);
        bool readInt64(uint64 &);
        bool readBigInteger(BIGNUM **);
        // used when reading a normal text string
        bool readString(std::string &);
        // used when reading A UTF string
        bool readWideString(std::wstring &, int enc = 0);
        bool readString(unsigned char *, uint32 &);
        // reads a string and allocates the memory for it.
        bool readString(unsigned char **, uint32 &);
        // reads data from the stream and writes it to the file.
        virtual bool readFile(FILE *, int);
        // sets the encoding to use
        bool SetEncoding(uint32 enc);
    private:
        uint32 m_nEncoding;     // the encoding used.
    };
};

#endif