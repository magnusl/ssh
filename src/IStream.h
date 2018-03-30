#ifndef _ISTREAM_H_
#define _ISTREAM_H_

namespace ssh
{
    class IStream
    {
    public:
        virtual int write(const unsigned char * , int)      = 0;
        virtual int read(const unsigned char *, int)        = 0;
        virtual void reset()                                = 0;
    };
};

#endif