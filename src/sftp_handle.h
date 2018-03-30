#ifndef _SFTP_HANDLE_H_
#define _SFTP_HANDLE_H_

#include "IStreamIO.h"
#include "types.h"

namespace sftp
{
    /*
     *
     */
    class sftp_handle
    {
    public:
        /*
         *
         */
        sftp_handle() {
            m_data = NULL;
        }

        /*
         *
         */
        ~sftp_handle() {
            delete [] m_data;
        }

        /*
         *
         */
        bool set(unsigned char * data, uint32 len) {
            m_data = new (std::nothrow) unsigned char[len];
            if(!m_data) return false;
            memcpy(m_data,data, len);
            m_len = len;
            return true;
        }

        bool read(ssh::IStreamIO * stream)
        {
            uint32 len;
            if(!stream->readInt32(len)) return false;
            m_data = new (std::nothrow) unsigned char[len];
            m_len = len;
            if(!m_data) return false;
            if(!stream->readBytes(m_data, len)) return false;
            return true;
        }

        /*
         *
         */
        bool write(ssh::IStreamIO * stream)
        {
            if(!stream->writeInt32(m_len) || !stream->writeBytes(m_data, m_len))
                return false;
            return true;
        }

    private:
        unsigned char * m_data;
        uint32 m_len;
    };
};

#endif