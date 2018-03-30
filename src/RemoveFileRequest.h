#ifndef _REMOVEFILEREQUEST_H_
#define _REMOVEFILEREQUEST_H_

#include "sftp_request_base.h"
#include "StatusMessage.h"

namespace sftp
{
    /* File:            RemoveFileRequest
     * Description:     Used to remove a file from the server.
     */
    class RemoveFileRequest : public sftp_request_base
    {
    public:

        // Constructor.
        RemoveFileRequest(const wchar_t * filename, sftp_impl * impl) : sftp_request_base(impl)
        {
            assert(filename != NULL);
            m_filename = filename;
        }

        // Constructor.
        RemoveFileRequest(const wstring & filename)
        {
            m_filename = filename;
        }

        /* Function:        write
         * Description:     Writes the request to the stream.
         */
        Status write(ssh::IStreamIO * stream, uint32 requestId)
        {
            // Write the message to the stream
            if(!stream->writeByte(SSH_FXP_REMOVE) ||
                !stream->writeInt32(requestId) ||
                !stream->writeWideString(m_filename))
            {
                std::cerr << "Could not write the remove request to the stream" << std::endl;
                return StatusError;
            }
            // Waiting for the reply.
            return StatusIncomplete;    
        }

        /* Function:        parse   
         * Descritpion:     Parses the reply.
         */
        Status parse(const sftp_hdr & hdr, ssh::IStreamIO * stream)
        {
            if(hdr.type != SSH_FXP_STATUS) {
                std::cerr << "Expected a SSH_FXP_STATUS message, got another" << std::endl;
                return SatatusError;
            }

            // read the status message.
            StatusMessage status;
            if(!status.read(stream)) {
                return StatusError;
            }

            // Now parse the status/error code.
            switch(status.code)
            {
            case SSH_FX_NO_SUCH_FILE:
            case SSH_FX_PERMISSION_DENIED:
                // The operation failed
                return StatusFailure;
            case SSH_FXP_OK:
                // The file was removed
                return StatusSuccess;
            default:
                std::cerr << "Unknown error code" << std::endl;
                return StatusError;
            }
        }

    protected:
        std::wstring m_filename;        // the file to remove.
    };
};

#endif