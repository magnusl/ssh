/* File:            sftp_dir_request
 * Description:     The sftp_dir_request is used to get a remote directory
 *                  listing.
 * Author:          Magnus Leksell
 *
 * Copyright © 2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/

#include "sftp_dir_request.h"
#include "ArrayStream.h"
#include "sftp_messages.h"
#include <string>
#include "definitions.h"
#include "sftp_errorcodes.h"
#include <iostream>
#include "sftp.h"

using namespace std;

namespace sftp
{
    /* Function:        sftp_dir_request::write_request
     * Description:     Writes the request to the strean.
     */
    int sftp_dir_request::write_request(ssh::IStreamIO * stream, uint32 request_id)
    {
        if(!stream->writeByte(SSH_FXP_READDIR) || !stream->writeInt32(request_id) || !m_handle->write(stream)) {
            cerr << "Error while writing the sftp_dir_request to the stream" << endl;
            return FATAL_ERROR;
        }
        return STATUS_SUCCESS;
    }
};