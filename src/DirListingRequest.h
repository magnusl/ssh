#ifndef _DIRLISTINGREQUEST_H_
#define _DIRLISTINGREQUEST_H_

#include "sftp_request_base.h"
#include "sftp_handle.h"
#include "sftp_directory.h"
#include "attributes_v3.h"
#include "sftp_impl.h"
#include "sftp_errorcodes.h"

namespace sftp
{
    /* Class:           DirListingRequest
     * Description:     Used to perform a directory listing. Currently supports version 3 & 4.
     */
    class DirListingRequest : public sftp_request_base
    {
    public:
        // Constructor
        DirListingRequest(sftp_handle * handle, sftp_directory * dir, sftp_impl * impl ) 
            : m_handle(handle), m_directory(dir), sftp_request_base(impl) 
        {
            assert(handle != NULL && dir != NULL);
        }

        /* Function:        write
         * Description:     Writes the request to the stream.
         */
        virtual sftp_request_base::Status write(ssh::IStreamIO * stream, uint32 requestId)
        {
            assert(stream != NULL);
            if(!stream->writeByte(SSH_FXP_READDIR) ||
                !stream->writeInt32(requestId) ||
                !m_handle->write(stream))
            {
                std::cerr << "Could not write the request to the stream" << std::endl;
                return StatusError;
            }
            return StatusIncomplete;
        }

        /* Function:        parse
         * Description:     Parses the reply.
         */
        virtual Status parse(const sftp_hdr & hdr, ssh::IStreamIO * stream)
        {
            if(hdr.type == SSH_FXP_NAME) {
                // Got one or more files
                uint32 count;
                if(!stream->readInt32(count)) {
                    std::cerr << "Could not read the number of name from the stream" << std::endl;
                    return StatusError;
                }
                std::wstring filename,              // the filename
                             longname;              // version 3 only
                file_attributes * attrs = NULL;     // the attributes, platform version specific.

                int version = m_impl->getVersion();
                // Now read all the files.
                for(uint32 i = 0;i < count;++i) {
                    // Allocate a new file attribute instance, using the current version.
                    attrs = m_impl->createAttributeInstance();
                    
                    if(!stream->readWideString(filename)) {
                        std::cerr << "Could not read the filename from the stream" << std::endl;
                        return StatusError;
                    }

                    // Only version < 4 has the longname field.
                    if(version == 3) {
                        if(!stream->readWideString(longname)) {
                            std::cerr << "Could not read the longname from the stream" << std::endl;
                            return StatusError;
                        }
                    }
                    // Now read the attributes
                    if(!attrs->read(stream)) {
                        std::cerr << "Could not read the attributes from the stream" << std::endl;
                    }
                    // Add the file to the directoty.
                    m_directory->addFile(filename, attrs);
                }
                return StatusIncomplete;
            } else if(hdr.type == SSH_FXP_STATUS) {
                // read the status message
                uint32 code;
                std::wstring error;
                if(!stream->readInt32(code) ||
                    !stream->readWideString(error)) 
                {
                    std::cerr << "Could not read the status message from the stream" << std::endl;
                    return StatusError;
                }
                switch(code)
                {
                case SSH_FX_EOF:
                    // All the file has been listed
                    //std::cerr << "Got a eof message" << std::endl;
                    return StatusSuccess;
                case SSH_FX_PERMISSION_DENIED:
                    // Permission denied
                    return StatusFailure;
                case SSH_FX_FAILURE:
                    // General failure.
                    return StatusFailure;
                default:
                    return StatusError;
                }
            } else {
                std::cerr << "Got a unexpected message" << std::endl;
                return StatusError;
            }
        }
    protected:
        sftp_handle     * m_handle;
        sftp_directory  * m_directory;
    };
};

#endif