#include <memory>
#include <cstdlib>
#include <algorithm>
#include <cmath>

#include "definitions.h"
#include "BoundedBuffer.h"

namespace ssh
{
    /* 
     *
     */
    BoundedBuffer::BoundedBuffer(uint32 size)
    {
        m_size = size;
        m_data = new (std::nothrow) unsigned char[size];
        m_in = m_out = m_count = 0;
    }

    /* Function:        BoundedBuffer::operator !
     * Description:     Checks if the buffer has been correctly initalized.
     */
    bool BoundedBuffer::operator !()
    {
        return m_data == NULL;
    }
    /* Function:        BoundedBuffer::read
     * Description:     Reads data from the bounded buffer.
     */
    int BoundedBuffer::read(unsigned char * buffer, int num)
    {
        if(num <= 0)
            return INVALID_ARGUMENT;
        if(m_count > 0)
        {
            /* have data to read */
            int read_count = std::min<uint32>(num, m_count);
            int toend = m_size - m_out;
            m_count -= read_count;
            if(toend < read_count) 
            {
                // will wrap around
                memcpy(buffer, &m_data[m_out], toend);
                read_count -= toend;
                memcpy(buffer + toend, m_data, read_count);
            }
            else
            {
                /* does not wrap */
                memcpy(buffer, m_data + m_out, read_count);
            }
            m_out = (m_out + read_count) % m_size;
            return read_count;
        }
        return 0;   
    }

    /* Function:        BoundedBuffer
     * Description:     Writes data to the bounded buffer.
     */
    int BoundedBuffer::write(const unsigned char * buffer, int num)
    {
        int nret;
        if(num < 0)
            return INVALID_ARGUMENT;

        if(m_count < m_size)
        {
            // space available
            int write_count = std::min<uint32>(m_size - m_count, num);
            nret = write_count;
            int toend = m_size - m_in;
            m_count += write_count;
            if(toend < write_count) {
                // will wrap, two writes required
                memcpy(m_data + m_in, buffer, toend);
                write_count -= toend;
                memcpy(m_data, buffer + toend, write_count);
                m_in = (m_in + write_count) % m_size;
            } else {
                // won't wrap
                memcpy(&m_data[m_in], buffer, write_count);
                m_in = (m_in + write_count) % m_size;
            }
            return nret;
        }
        return 0;
    }
};