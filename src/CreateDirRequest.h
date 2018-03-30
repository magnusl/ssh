#ifndef _CREATEDIRREQUEST_H_
#define _CREATEDIRREQUEST_H_

#include "sftp_request_base.h"
#include "sftp_messages.h"
#include <iostream>
#include <string>

namespace sftp
{       
    /* Class:           CreateDirRequest
     * Description:     Creates a directory
     */
    class CreateDirRequest : public sftp_request_base
    {
    public:

        CreateDirRequest(const wchar_t * path,sftp_impl * impl) :
          sftp_request_base(impl), m_path(path)
          {
          }

          CreateDirRequest(const std::wstring & path,sftp_impl * impl) :
          sftp_request_base(impl), m_path(path)
          {
          }

        /* Function:        write
         * Description:     Writes the request to a file.
         */
        Status write(ssh::IStreamIO * stream, uint32 requestId, sftp_impl * impl) : sftp_request_base(impl)
        {
            assert(stream != NULL);
            if(!stream->writeByte(SSH_FXP_MKDIR) ||
                !stream->writeInt32(requestId) ||
                !stream->writeString(m_path) ||
                !m_attrs->write(stream))
            {
                std::cerr << "Could not write the SSH_FXP_MKDIR request to the stream" << std::endl;
                return StatusError;
            }
            return StatusIncomplete;
        }

        /* Function:        parse
         * Description:     Parses the reply.
         */
        Status parse(const sftp_hdr & hdr, ssh::IStreamIO * stream)
        {
            if(hdr.type != SSH_FXP_STATUS) {
                std::cerr << "Didn't get the expected Status message" << std::endl;
                return StatusError;
            }
            StatusMessage status;
            if(!status.read(stream)) {
                std::cerr << "Could not read the status message from the stream" << std::endl;
                return StatusError;
            }

            switch(hdr.code)
            {
            case SSH_FX_OK:
                // the directory was created
                return StatusSuccess;
            case SSH_FX_PERMISSION_DENIED:
                // permission denied
                return StatusFailure;
            case SSH_FX_FAILURE:
                // could not create the directory
                return StatusFailure;
            default:
                std::cerr << "CreateDirRequest::parse -> unknown error code" << std::endl;
                return StatusError;
            }
        }

    };
};  

#endif