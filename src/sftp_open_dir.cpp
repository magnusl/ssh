/* File:            sftp_open_dir.cpp
 * Description:     The sftp_open_dir request is used when the client wants
 *                  to perform a operation on a directory.
 * Author:          Magnus Leksell
 *
 * Copyright © 2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/

#include "sftp_open_dir.h"
#include "ArrayStream.h"
#include "sftp_messages.h"
#include "definitions.h"
#include <assert.h>
#include <iostream>
#include "sftp.h"

using namespace std;

namespace sftp
{
    /* Function:        sftp_open_dir::sftp_open_dir
     * Description:     Initalizes the request.
     */
    sftp_open_dir::sftp_open_dir(sftp_raw_notify * notify, const wchar_t * dir, sftp_handle * handle) : sftp_request(notify),
            m_path(dir), m_handle(handle) {
        assert(handle != NULL);
        assert(dir != NULL);
    }

    /* Function:        sftp_open_dir::write_request
     * Description:     Writes the request to the buffer.
     */
    int sftp_open_dir::write_request(ssh::IStreamIO * stream , uint32 request_id)
    {
        if(!stream->writeByte(SSH_FXP_OPENDIR) || !stream->writeInt32(request_id) || !stream->writeStringUTF8(m_path))
        {
            cerr << "sftp_open_dir::write_request: could not write the request" << endl;
            return STATUS_FAILURE;
        }
        return STATUS_SUCCESS;
    }

    /* Function:        sftp_open_dir::process_data
     * Description:     Processes the reply sent by the server.
     */
    int sftp_open_dir::parse_reply(unsigned char type,  ssh::IStreamIO * stream)
    {
        switch(type)
        {
        case SSH_FXP_HANDLE:
            {
                unsigned char handle[1024]; 
                uint32 len;
            /*  
                byte   SSH_FXP_HANDLE
                uint32 request-id
                string handle
            */
                if(!stream->readInt32(len)) {
                    cerr << "Failed to read the length of the SFTP handle" << endl;
                    return STATUS_FAILURE;
                }
                if(len == 0 || len > 1024) {
                    cerr << "Invalid handle length" << endl;
                    return STATUS_FAILURE;
                }
                // read the handle.
                if(!stream->readBytes(handle, len)) {
                    cerr << "Failed to read the handle" << endl;
                    return STATUS_FAILURE;
                }
                // set the handle
                if(!m_handle->set(handle, len))
                    return STATUS_FAILURE;
                
                if(m_notify) m_notify->OnOpenDirectorySuccess();
            }
            break;
        case SSH_FXP_STATUS:
            uint32 reason = 0;
            if(m_notify) m_notify->OnOpenDirectoryFailure(reason);
            break;

        }
        return SFTP_REQUEST_COMPLETE;
    }


};