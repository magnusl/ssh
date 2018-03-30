#ifndef _SFTP_OPERATION_H_
#define _SFTP_OPERATION_H_

#include "sftp_notify.h"
#include "IStreamIO.h"
#include "sftp_impl.h"
#include "sftp_hdr.h"
#include "ArrayStream.h"
#include "types.h"

#define OPERATION_FAILED        -13

namespace sftp
{
    /* Class:           sftp_operation
     * Description:     Baseclass for a SFTP operation, like filetransfer, directory listing etc.
     */
    class sftp_operation
    {
    public:
        sftp_operation(sftp_notify * notify, sftp_impl * impl) 
            : m_notify(notify) , m_impl(impl) 
        {
        }

        virtual ~sftp_operation() {}

        // The different status
        enum OperationStatus {OperationPending = 0, OperationSuccess, OperationFailure, OperationError, OperationWaiting,
            OperationNewPacket, OperationWorkingOnPacket, OperationAborted};
        // Handles the reply for the operation.
        virtual OperationStatus handle_reply(const sftp_hdr & hdr, ssh::IStreamIO *) = 0;
        // writes a message to the buffer.
        virtual OperationStatus write(ssh::ArrayStream *, uint32) = 0;
    protected:
        sftp::sftp_impl * m_impl;
        sftp_notify * m_notify;

    };
};

#endif