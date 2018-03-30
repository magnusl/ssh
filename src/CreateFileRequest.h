#ifndef _CREATEFILEREQUEST_H_
#define _CREATEFILEREQUEST_H_

#include "sftp_request_base.h"
#include "file_attributes.h"
#include <string>
#include "sftp_handle.h"
#include "StatusMessage.h"
#include <iostream>
#include "sftp_messages.h"
#include "sftp_errorcodes.h"
#include "sftp_file_modes.h"

namespace sftp
{
    /* Class:           CreateFileRequest
     * Description:     Used to create a file on the remote host.
     */
    class CreateFileRequest : public sftp_request_base
    {
    public:
        CreateFileRequest(const wchar_t * path, 
            const file_attributes * attrs, 
            sftp_handle & handle,
            sftp_impl * impl,
            bool bOverWrite = false)  
            : sftp_request_base(impl),m_handle(handle)
        {
            m_filepath      = path;
            m_attrs         = attrs;
            m_bOverWrite    = bOverWrite;
        }

        /* Function:        write   
         * Description:     Writes the request to the stream.
         */
        Status write(ssh::IStreamIO * stream, uint32 requestId)
        {
            // Set the flags
            uint32 flags =  SSH_FXF_WRITE | SSH_FXF_CREAT | (m_bOverWrite ? SSH_FXF_TRUNC : SSH_FXF_EXCL);
            if(!stream->writeByte(SSH_FXP_OPEN) ||
                !stream->writeInt32(requestId) ||
                !stream->writeWideString(m_filepath) ||
                !stream->writeInt32(flags) ||
                !m_attrs->write(stream))
            {
                std::cerr << "Could not write the create file request to the stream" << std::endl;
                return StatusError;
            }
            return StatusIncomplete;
        }

        /* Function:        parse
         * Description:     Parses the reply.
         */
        Status parse(const sftp_hdr & hdr, ssh::IStreamIO * stream)
        {
            if(hdr.type == SSH_FXP_HANDLE) {
                // The file was created and opened.
                if(!m_handle.read(stream)) {
                    std::cerr << "Could not read the file handle from the stream" << std::endl;
                    return StatusError;
                }
                return StatusSuccess;
            } else if(hdr.type == SSH_FXP_STATUS) {
                StatusMessage status;
                if(!status.read(stream)) {
                    std::cerr << "Could not read the status message from the stream" << std::endl;
                    return StatusError;
                }
                // Set the error code.
                SetErrorString(status.msg.c_str());
                switch(status.code) 
                {
                case SSH_FX_PERMISSION_DENIED:
                    return StatusFailure;
                case SSH_FX_FAILURE:
                    // assume it means that the file already exists
                    return StatusFailure;
                case SSH_FX_NO_SUCH_FILE:
                    return StatusFailure;
                default:
                    return StatusError;
                }
            } else {
                std::cerr << "Unexpected message, was expecting a SSH_FXP_HANDLE or a SSH_FXP_STATUS message" << std::endl;
                return StatusError;
            }
        }

    private:

        sftp_handle &               m_handle;
        std::wstring                m_filepath;
        const file_attributes   *   m_attrs;
        bool                        m_bOverWrite;

    };
};

#endif