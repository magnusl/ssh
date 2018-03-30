#ifndef _SFTP_FILE_TRANSFER_H_
#define _SFTP_FILE_TRANSFER_H_

#include "sftp.h"
#include "sftp_notify.h"
#include "sftp_handle.h"
#include <cstdio>
#include "sftp_definitions.h"

namespace sftp
{
    /* Class:           sftp_file_transfer
     * Description:     Used to transfer files over the SFTP protocol.
     */
    class sftp_file_transfer : public sftp_raw_notify
    {
    public:
        sftp_file_transfer(sftp_connection * con, sftp_notify * notify) : m_con(con), m_notify(notify) 
        {
            m_transfered = 0;
            m_buff = 0;
        }

        virtual ~sftp_file_transfer() {
            if(m_buff) delete [] m_buff;
        }

        virtual void cleanup() = 0;

        /* This functions is called by the application, and should therefore be thread synchronized */
        virtual bool beginTransfer(const wchar_t *, const wchar_t *) = 0;
        virtual bool pause() = 0; 
        virtual bool stop() = 0;
        virtual bool resume() = 0;

        /* These functions should never be called by the application */
        void OnOpenFileFailure(uint32);
        void OnClose();
        void OnCloseFailure();

        virtual bool BeginTransfer(const wchar_t * localfile, const wchar_t * remotefile) = 0;
    protected:
        sftp_notify *           m_notify;
        sftp_connection *       m_con;
        std::wstring            m_localfile, m_remotefile;
        sftp_handle             m_handle;
        time_t                  m_start;                
        uint64                  m_size;
        uint64                  m_transfered;
        unsigned char *         m_buff;         // the buffer.
        uint32                  m_bufflen;      // the length of the buffer.
    };
};

#endif