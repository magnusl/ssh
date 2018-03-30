#include "sftp_close_request.h"
#include <iostream>

using namespace std;

namespace sftp
{
    /* Function:        sftp_close_request::write_request
     * Description:     Writes the close request to the stream.
     */
    int sftp_close_request::write_request(ssh::IStreamIO * stream, uint32 request_id)
    {
        if(!stream->writeByte(SSH_FXP_CLOSE) || !stream->writeInt32(request_id) ||
            !m_handle->write(stream))
        {
            cerr << __FUNCTION__ <<": Failed to write the request to the stream" << endl;
            return FATAL_ERROR;
        }
        return STATUS_SUCCESS;
    }

    /* Function:        sftp_close_request::parse_reply
     * Description:     Handles the replyt to the close request.
     */
    int sftp_close_request::parse_reply(unsigned char type, ssh::IStreamIO * stream)
    {
        if(type == SSH_FXP_STATUS)
        {
            uint32 code;
            wstring reason;
            string languages;
            if(!stream->readInt32(code) || !stream->readStringUTF8(reason) || 
                !stream->readString(languages))
            {
                cerr << __FUNCTION__ << ": Failed to parse the SSH_FXP_STATUS message" << endl;
                return FATAL_ERROR;
            }
    
            if(m_notify) m_notify->OnClose();
            return SFTP_REQUEST_COMPLETE;
        } 
        else
        {
            cerr << __FUNCTION__ <<": unknown reply to the SSH_FXP_CLOSE request" << endl;
            return STATUS_FAILURE;
        }
    }
};