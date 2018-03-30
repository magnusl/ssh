#ifndef _SFTP_SEND_FILE_H_
#define _SFTP_SEND_FILE_H_

#include "sftp_file_transfer.h"
#include <fstream>

#define SFTP_TRANSFER_BUFFER_SIZE (1024*1024)

namespace sftp
{
    /* Class:           sftp_send_file
     * Description:     Sends a file.
     */
    class sftp_send_file : public sftp_file_transfer
    {
    public:
        sftp_send_file(sftp_connection * con, 
                       sftp_notify * notify,
                       bool binary = true);
        ~sftp_send_file();
        void OnWriteSuccess(uint32 num);
        void OnWriteFailure(uint32 reason);
        bool start(const wchar_t *, const wchar_t *);
        void OnOpenFileSuccess();
        bool BeginTransfer(const wchar_t * localfile, const wchar_t * remotefile);
        bool pause();
        bool resume();
        bool stop();
    protected:
        void sendData();
        std::wifstream m_input;
    };
};

#endif