#ifndef _SFTP_MKDIR_H_
#define _SFTP_MKDIR_H_

#include "sftp_operation.h"
#include <string>
#include "CreateDirRequest.h"

namespace sftp
{
    /* Class:           sftp_mkdir
     * Description:     Used to create a directory on the remote host
     */
    class sftp_mkdir : sftp_operation
    {
    public:
        sftp_mkdir(const wchar_t * path, sftp_notify * notify, sftp_impl * impl) 
            : sftp_operation(notify, impl), m_path(path),m_request(0) 
        {
        }

        sftp_mkdir(const std::wstring & path, sftp_notify * notify, sftp_impl * impl) 
             : sftp_operation(notify, impl), m_path(path), m_request(0)
        {
        }

        OperationStatus handle_reply(const sftp_hdr & hdr, ssh::IStreamIO *)
        {
            return StatusError;
        }

        // writes a message to the buffer.
        OperationStatus write(ssh::ArrayStream * stream, uint32 id)
        {
            m_request = new (std::nothrow) CreateDirRequest(m_path,m_impl);
            if(m_request == NULL) {
                std::cerr << "Could not create the CreateDirRequest instance" << std::endl;
                return OperationError;
            }
            sftp_request_base status = m_request->write(stream,id);
        }

    private:
        std::wstring m_path;
        CreateDirRequest * m_request;

    };
};

#endif