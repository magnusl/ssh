#ifndef _OPENFILEREQUEST_H_
#define _OPENFILEREQUEST_H_

#include "sftp_request_base.h"
#include "file_attributes.h"
#include "attributes_v3.h"
#include "sftp_messages.h"
#include <iostream>

#include "sftp_file_modes.h"
#include "sftp_handle.h"

namespace sftp
{
    /* Class:           OpenFileRequest
     * Description:     Used to open a file.
     */
    class OpenFileRequest : public sftp_request_base
    {
    public:
        OpenFileRequest(const wchar_t * filename, sftp_handle * handle, sftp_impl * impl) :
          sftp_request_base(impl)
        {
            m_path          = filename;
            m_handle        = handle;
            m_flags         = 0;
            m_attributes    = NULL;
        }

        /* Function:        write
         * Description:     Writes the request to the stream
         */
        virtual sftp_request_base::Status write(ssh::IStreamIO * stream, uint32 requestId)
        {
            // The default attributes, used when opening a already existing file.
            attributes_v3 default_attributes;
            // Write the data to the stream.
            if(!stream->writeByte(SSH_FXP_OPEN) ||      // Type
                !stream->writeInt32(requestId) ||       // request id
                !stream->writeWideString(m_path) ||     // the path
                !stream->writeInt32(m_flags) ||         // the flags
                !default_attributes.write(stream))  
            {
                std::cerr << "Could not write the request to the stream" << std::endl;
                return StatusError;
            }
            return StatusIncomplete;
        }

        /* Function:        parse
         * Description:     Parses the reply.
         */
        virtual sftp_request_base::Status parse(const sftp_hdr & hdr, ssh::IStreamIO * stream)
        {
            if(hdr.type == SSH_FXP_HANDLE) {
                if(!m_handle->read(stream)) {
                    std::cerr << "Could not read the file handle from the stream" << std::endl;
                    return StatusError;
                }
                // The request was successful.
                return StatusSuccess;
            } else if(hdr.type == SSH_FXP_STATUS) {
                StatusMessage status;
                if(!status.read(stream)) {
                    std::cerr << "could not read the status message from the stream" << std::endl;
                    return StatusError;
                }

                SetErrorString(status.msg.c_str());
                return StatusFailure;
            } else {
                std::cerr << "Got a unknown reply message" << std::endl;
                return StatusError;
            }
        }

        // Modifiers
        OpenFileRequest & Write()   {m_flags |= SSH_FXF_READ; return *this;}
        OpenFileRequest & Read()    {m_flags |= SSH_FXF_WRITE; return *this;}
        OpenFileRequest & Text()    {return *this;}
        OpenFileRequest & Append()  {m_flags |= SSH_FXF_APPEND; return *this;}
        OpenFileRequest & Create(file_attributes * attrs) {m_flags |= SSH_FXF_CREAT; m_attributes = attrs; return *this;}
        OpenFileRequest & Trunc()   {m_flags |= SSH_FXF_TRUNC; return *this;}


    protected:
        std::wstring        m_path;         // The path to the file
        sftp_handle   *     m_handle;       // the file handle.
        uint32              m_flags;
        file_attributes*    m_attributes;
    };
};

#endif