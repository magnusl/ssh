#ifndef _FILESTATREQUEST_H_
#define _FILESTATREQUEST_H_

#include "sftp_request_base.h"
#include "file_attributes.h"
#include "StatusMessage.h"
#include <assert.h>
#include "sftp_messages.h"

namespace sftp
{
    /* Class:       FileStatRequest
     *
     */
    class FileStatRequest : public sftp_request_base
    {
    public:
        /*
         * fstat
         */
        FileStatRequest(sftp_handle * handle, file_attributes * attrs, sftp_impl * impl) 
            : sftp_request_base(impl), m_handle(handle), m_attrs(attrs) 
        {
            assert(m_handle != NULL);
        }

        /*
         *  // defaults to stat, true -> lstat  
         */
        FileStatRequest(const wchar_t * path, file_attributes * attrs, sftp_impl * impl, bool FollowSymbolicLinks = false) 
            : sftp_request_base(impl), m_path(path), m_FollowSymbolicLinks(FollowSymbolicLinks) , m_attrs(attrs), m_handle(0)
        {
            assert(path != NULL);
        }

        /* Function:        write
         * Description:     Writes the request to the stream
         */
        Status write(ssh::IStreamIO * stream, uint32 id)
        {
            unsigned char type = (m_handle != 0 ? SSH_FXP_FSTAT : (m_FollowSymbolicLinks ? SSH_FXP_LSTAT : SSH_FXP_STAT));
            if(!stream->writeByte(type) ||
                !stream->writeInt32(id)) 
            {
                std::cerr << "Could not write the packet header to the stream" << std::endl;
                return StatusError;
            }
            if(type == SSH_FXP_FSTAT) {
                if(!m_handle->write(stream)) {
                    std::cerr << "Could not write the file handle to the stream" << std::endl;
                    return StatusError;
                }
            } else {
                if(!stream->writeWideString(m_path)) {
                    std::cerr << "Could not write the file path to the stream" << std::endl;
                    return StatusError;
                }
            }
            // what version?
            if(m_attrs->getVersion() >= 4) {
                // write the flags
                if(!stream->writeInt32(0)) {
                    std::cerr << "Could not write the flag to the stream" << std::endl;
                    return StatusError;
                }
            }
            // Wait for the reply.
            return StatusIncomplete;
        }

        /* Function:        parse
         * Description:     Parses the reply from the server.
         */
        Status parse(const sftp_hdr & hdr, ssh::IStreamIO * stream)
        {
            if(hdr.type == SSH_FXP_ATTRS)
            {
                // The request was successfull.
                if(!m_attrs->read(stream)) {
                    // could not read the attributes from the stream
                    std::cerr << "Could not read the file attributes from the stream" << std::endl;
                    return StatusError;
                }
                return StatusSuccess;
            } else if(hdr.type == SSH_FXP_STATUS) {
                // The request failed
                StatusMessage status;
                if(!status.read(stream)) {
                    std::cerr << "Could not read the status message from the stream" << std::endl;
                    return StatusError;
                } else {
                    // Set the error message
                    SetErrorString(status.msg.c_str());
                    return StatusFailure;
                }
            } else {
                // Unknown packet.
                std::cerr << "Unexpected reply to the file stat message" << std::endl;
                return StatusFailure;
            }
        }

    protected:
        std::wstring    m_path;
        sftp_handle *   m_handle;
        file_attributes * m_attrs;
        bool m_FollowSymbolicLinks;
    };
};

#endif