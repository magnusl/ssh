#ifndef _BOUNDEDBUFFER_H_
#define _BOUNDEDBUFFER_H_

#include "types.h"

namespace ssh
{
    /* Class:           BoundedBuffer
     * Description:     A bounded buffer implementation.
     */
    class BoundedBuffer
    {
    public:
        BoundedBuffer(uint32);
        bool operator!();
        int read(unsigned char *, int);
        int write(const unsigned char *, int);
        uint32 usage() {return m_count;}
        uint32 size() {return m_size;}
    protected:
        uint32 m_size, m_in, m_out, m_count;
        unsigned char * m_data;
    };
};
#endif