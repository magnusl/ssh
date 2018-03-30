#ifndef _SFTP_SEND_H_
#define _SFTP_SEND_H_

#include "sftp_transfer.h"
#include "sftp_request_base.h"
#include <memory>               // for std::auto_ptr
#include "sftp_notify.h"
#include "sftp_impl.h"
#include <vector>

namespace sftp
{
    /* Class:           sftp_send
     * Description:     Used to send files to the server
     */
    class sftp_send : public sftp_transfer
    {
    public:
        sftp_send(const std::vector<FileTransferElement> files,     // the files.
                  sftp_notify * notify,                             // who to notify
                  sftp_impl * impl);                                // the current implementation.
        
        // destructor.
        ~sftp_send();

        enum SendState {SendInit = 0, EmptyFile, CreatingFile, FileCreated, NewPacket, 
            PacketInProgress, FileWritten, PacketWritten, ClosingFailure, ClosingSuccess};
        // Handles the reply for the operation.
        OperationStatus handle_reply(const sftp_hdr & hdr, ssh::IStreamIO *);
        // writes a message to the buffer.
        OperationStatus write(ssh::ArrayStream *,uint32);

    private:
        sftp_request_base * m_request;                      // the current request
        uint32 fileIt;  // the current file
        FILE * m_file;
        uint64 m_bytesWritten, m_fileSize, m_packetBytesSent, m_packetSize;
        SendState m_state;
        file_attributes * m_attribs;
        sftp_handle m_handle;
        time_t m_transferStart;
    };
};

#endif