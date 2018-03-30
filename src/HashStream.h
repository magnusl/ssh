#ifndef _HASHSTREAM_H_
#define _HASHSTREAM_H_

#include "IStreamIO.h"
#include "md5.h"
#include "sha1.h"

namespace ssh
{   
    /* Function:        HashStream
     * Description:     
     */
    class HashStream : public IStreamIO
    {
    public:

        // initalize the "stream"
        HashStream(const char * hash)
        {
            if(strcmp(hash, "md5") == 0)        m_instance = new (std::nothrow) md5;
            else if(strcmp(hash, "sha1") == 0)  m_instance = new (std::nothrow) sha1;
            else                                m_instance = NULL;
        }

        // perform the required cleanup.
        ~HashStream()
        {
            delete m_instance;
        }

        // checks the status.
        bool operator!()
        {
            return m_instance == NULL;
        }

        /* Function:        writeBytes
         * Description:     Updates the hash with the data.
         */
        bool writeBytes(const unsigned char * src, uint32 num)
        {
            if(m_instance) {
                m_instance->update(src, num);
                return true;
            }
            return false;
        }
    
        /* Function:        readBytes
         * Description:     n/a
         */
        bool readBytes(unsigned char *, uint32)
        {
            // Can't read data from this stream
            return false;
        }

        /* Function:        digest
         * Description:     Finalizes the digest. Can't call writeBytes after this function without
         *                  calling clear in between.
         */
        void digest(unsigned char * dst, uint32 * num) 
        {
            if(m_instance) m_instance->finalize(dst, num);
        }

        /* Function:        clear
         * Description:     resets the hash.
         */
        void clear()
        {
            if(m_instance) m_instance->reinit();
        }

    protected:
        HashBase * m_instance;
    };

};

#endif