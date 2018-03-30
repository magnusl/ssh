#ifndef _REQUESTOPERATION_H_
#define _REQUESTOPERATION_H_

#include "sftp_operation.h"
#include "sftp_request_base.h"

namespace sftp
{
    /* Class:           RequestOperation
     * Description:     Used to perform a single request.
     */
    class RequestOperation : public sftp_operation
    {
        RequestOperation(sftp_request_base * request,sftp_notify * notify, sftp_impl * impl)
            : sftp_operation(notify, impl), m_request(request)
        {
        }

        /* Returns the status of the operation */
        virtual int status() {return 0;}

        // Handles the reply for the operation.
        OperationStatus handle_reply(const sftp_hdr & hdr, ssh::IStreamIO *)
        {
            if(m_request == NULL) return OperationError;
            int nret = m_request->parse(stream,id);
            if(nret == sftp_request_base::StatusSuccess)
                return OperationSuccess;
            else if(nret == sftp_request_base::StatusFailure)
                return OperationFailure;
            else
                return OperationError;
        }

        // writes a message to the buffer.
        OperationStatus write(ssh::ArrayStream * stream, uint32 id)
        {
            if(m_request == NULL) return OperationError;
            sftp_request_base::Status status = m_request->write(stream,id);
            switch(status)
            {
            case sftp_request_base::StatusSuccess:
                return OperationSuccess;
            case StatusIncomplete:
                return OperationPending;
            case sftp_request_base::StatusFailure:
                return OperationFailure;
            default:
                return OperationError;
            }
        }

    private:
        sftp_request_base * m_request;
    };
};

#endif