#ifndef _ARRAYSTREAM_H_
#define _ARRAYSTREAM_H_

#include "IStreamIO.h"
#include "types.h"

namespace ssh
{
    /* Class:           ArrayStream
     * Description:     Wraps the access to a array.
     */
    class ArrayStream : public IStreamIO
    {
    public:
        /* Function:        ArrayStream::ArrayStream
         * Description:     Constructor, initalizes the instance.
         */
        ArrayStream(unsigned char * data, uint32 len) 
        {
            m_length = len;
            m_readpos = m_writepos = 0;
            m_data = data;
        }

        /* Function:        writeBytes
         * Description:     Writes data to the array.
         */
        bool writeBytes(const unsigned char * data, uint32 num)
        {
            if((m_writepos + num) > m_length)
                return false;
            memcpy(m_data + m_writepos, data, num);
            m_writepos += num;
            return true;
        }

        /* Function:        
         * Description:
         */
        bool readBytes(unsigned char * data, uint32 num)
        {
            if((m_readpos + num) > m_length)
                return false;
            memcpy(data, m_data + m_readpos, num);
            m_readpos += num;
            return true;
        }

        // reads data to the file from the stream
        bool readFile(FILE * file, int count)
        {
            if((m_readpos + count) > m_length)
                return false;
            if(fwrite(m_data + m_readpos, count, 1, file) != 1)
                return false;
            m_readpos += count;
            return true;
        }

        // Writes data from the file to the stream.
        bool writeFile(FILE * file, int count)
        {
            if((m_writepos + count) > m_length)
                return false;
            if(fread(m_data + m_writepos, count, 1, file) != 1)
                return false;
            m_writepos += count;
            return true;
        }
    
        uint32 getFreeSpace() {return m_length - m_writepos;}
        uint32 usage() {return m_writepos;}
    private:
        uint32 m_length, m_readpos, m_writepos;
        unsigned char * m_data;
    };

    /*
     *
     */
    class ArrayStream_const : public IStreamIO
    {
    public:
        /* Function:        ArrayStream::ArrayStream
         * Description:     Constructor, initalizes the instance.
         */
        ArrayStream_const(const unsigned char * data, uint32 len) 
        {
            m_length = len;
            m_readpos = m_writepos = 0;
            m_data = data;
        }

        /* Function:        writeBytes
         * Description:     Writes data to the array.
         */
        bool writeBytes(const unsigned char * data, uint32 num)
        {
            return false;
        }

        /* Function:        
         * Description:
         */
        bool readBytes(unsigned char * data, uint32 num)
        {
            if((m_readpos + num) > m_length)
                return false;
            memcpy(data, m_data + m_readpos, num);
            m_readpos += num;
            return true;
        }
    
        uint32 getFreeSpace() {return m_length - m_writepos;}
        uint32 usage() {return m_writepos;}
    private:
        uint32 m_length, m_readpos, m_writepos;
        const unsigned char * m_data;
    };
};
#endif