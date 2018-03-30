#ifndef _SFTP_TRANSFER_H_
#define _SFTP_TRANSFER_H_

#include "types.h"
#include "sftp_operation.h"
#include "sftp_handle.h"
#include "sftp_notify.h"
#include <vector>
#include "FileTransferElement.h"
#include <list>
#include "sftp_impl.h"

namespace sftp
{
    /* Class:           sftp_transfer
     * Description:     Used to transfer a file between the client and server (send or recv).
     */
    class sftp_transfer : public sftp_operation
    {
    public:
        sftp_transfer(const std::vector<FileTransferElement> files,         // the files to transfer
            sftp_notify * notify,                                           // who to notify
            sftp_impl * impl) :                                             // the implementation
          sftp_operation(notify, impl), m_files(files), m_impl(impl)
          {
          }

    protected:
        uint32 m_state;                         
        const std::vector<FileTransferElement> m_files;     // The files to transfer.
        std::list<FileTransferElement>::const_iterator it;  // points to the current file.
        sftp_impl * m_impl;
    };
};


#endif