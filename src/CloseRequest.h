/* File:                CloseRequest.h
 * Description:         Implements the request used to close files and 
 *                      directories.
 * Author:              Magnus Leksell
 *
 * Copyright © 2007 Magnus Leksell, all rights reserved
 *****************************************************************************/

#ifndef _CLOSEREQUEST_H_
#define _CLOSEREQUEST_H_

#include "sftp_handle.h"
#include "sftp_request_base.h"
#include "sftp_messages.h"
#include <iostream>
#include <assert.h>
#include "sftp_errorcodes.h"

namespace sftp
{
    /* Class:           CloseRequest
     * Description:     Used to close a open file or directory.
     */
    class CloseRequest : public sftp_request_base
    {
    public:
        CloseRequest(sftp_handle * handle, sftp_impl * impl) : sftp_request_base(impl)
        {
            assert(handle != NULL);
            m_handle = handle;
        }

        virtual ~CloseRequest()
        {
        }

        /* Function:        write   
         * Description:     Writes the request to the stream.
         */
        virtual sftp_request_base::Status write(ssh::IStreamIO * stream, uint32 requestId)
        {
            if(!stream->writeByte(SSH_FXP_CLOSE) || !stream->writeInt32(requestId) || !m_handle->write(stream)) {
                std::cerr << "Could not write the request to the stream" << std::endl;
                return StatusError;
            }
            return StatusIncomplete;
        }

        /* Function:        parse
         * Description:     Parses the request reply.
         */
        virtual sftp_request_base::Status parse(const sftp_hdr & hdr, ssh::IStreamIO * stream)
        {
            if(hdr.type != SSH_FXP_STATUS) {
                std::cerr << "Didn't get the expected reply" << std::endl;
                return StatusError;
            }
            uint32 code;
            if(!stream->readInt32(code)) {
                std::cerr << "Could not read the status code from the stream" << std::endl;
                return StatusError;
            }
            // A close request may fail.
            return (code == SSH_FX_OK ? StatusSuccess : StatusFailure);
        }

    private:
        sftp_handle * m_handle;
    };
};

#endif