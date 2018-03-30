#ifndef _OPENDIRREQUEST_H_
#define _OPENDIRREQUEST_H_

#include "sftp_request_base.h"
#include "sftp_handle.h"
#include "sftp_messages.h"

namespace sftp
{
    /* Class:           OpenDirRequest
     * Description:     Used to open a directory.
     */
    class OpenDirRequest : public sftp_request_base
    {
    public:
        OpenDirRequest(const wchar_t * path, sftp::sftp_handle * handle, sftp_impl * impl) : sftp_request_base(impl)
        {
            m_path      = path;
            m_handle    = handle;
        }
 
        virtual ~OpenDirRequest()
        {
        }

        /* Function:        write
         * Description:     Writes the request to a stream
         */
        virtual sftp_request_base::Status write(ssh::IStreamIO * stream, uint32 requestId)
        {
            assert(stream != NULL);
            if(!stream->writeByte(SSH_FXP_OPENDIR) ||       // write the message type
                !stream->writeInt32(requestId) ||           // write the request id
                !stream->writeWideString(m_path))           // write the directory path
            {
                std::cerr << "Could not write the request to the stream" << std::endl;
                return StatusError;
            }
            return StatusIncomplete;
        }

        /* Function:        sftp_request_base::Status parse
         * Description:     Parses the request reply.
         */
        virtual sftp_request_base::Status parse(const sftp_hdr & hdr, ssh::IStreamIO * stream)
        {
            if(hdr.type == SSH_FXP_HANDLE) {
                if(!m_handle->read(stream)) {
                    std::cerr << "Could not read the directory handle from the stream" << std::endl;
                    return StatusError;
                }
                // The directory was opened.
                return StatusSuccess;
            } else if(hdr.type == SSH_FXP_STATUS) {
                // could not open the directory
                return StatusFailure;
            } else {
                std::cerr << "Didn't get a expected packet" << std::endl;
                return StatusError;
            }
        }

    private:
        sftp_handle * m_handle;
        std::wstring m_path;
    };
};


#endif