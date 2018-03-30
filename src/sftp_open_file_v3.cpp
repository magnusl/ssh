#include "sftp_open_file_v3.h"
#include "sftp_messages.h"
#include "definitions.h"
#include <iostream>

using namespace std;

namespace sftp
{
    /* Function:        sftp_open_file_v3::write_request
     * Description:     Writes the SSH_FXP_OPEN request to the stream. SFTP version 3
     *                  implementation.
     */
    int sftp_open_file_v3::write_request(ssh::IStreamIO * stream, uint32 request)
    {
        if(!stream->writeByte(SSH_FXP_OPEN) || !stream->writeInt32(request) ||
            !stream->writeStringUTF8(m_filename) || !stream->writeInt32(pflags) ||
            !attributes.write(stream))
        {
            cerr << __FUNCTION__ << "Faild to write the request to the stream" << endl;
            return FATAL_ERROR;
        }
        return STATUS_SUCCESS;
    }   
};