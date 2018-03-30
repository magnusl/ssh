#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "IStreamIO.h"
#include "types.h"

namespace ssh
{

    /* Class:           Buffer
     * Description:     A databuffer.
     */
    class Buffer : public IStreamIO
    {
    public:
        Buffer(uint32);
        ~Buffer();

        /* Functions to satisfy the IStream Interface */
        bool writeBytes(const unsigned char * ,uint32);
        bool readBytes(unsigned char *, uint32);
        void reset();   
        bool operator !() {return data == 0;}

        uint32 usage() {return write_pos;}

        unsigned char * data;
    protected:
        uint32 read_pos, write_pos, size;
    };
};

#endif