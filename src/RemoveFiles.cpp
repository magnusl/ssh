#include "RemoveFiles.h"
#include <iostream>
#include "StatusMessage.h"
#include "assert.h"
#include "sftp_messages.h"
#include "sftp_errorcodes.h"

using namespace std;

namespace sftp
{
    /* Function:        RemoveFiles::RemoveFiles
     * Description:     Constructor, performs the required initalization.
     */
    RemoveFiles::RemoveFiles(const std::list<std::wstring> & files, sftp_notify * notify, sftp_impl * impl) 
        : sftp_operation(notify, impl)
    {
        // Copy the files.
        m_files = files;
    }

    /*
     *
     */
    sftp_operation::OperationStatus RemoveFiles::write(ssh::ArrayStream * stream, uint32 requestId)
    {
        assert(m_files.empty() != true);
        if(m_files.empty()) return OperationError;  // Should not happen.
        // Get the first file
        file = m_files.front(); m_files.pop_front();
        // Write the request to the stream.
        if(!stream->writeByte(SSH_FXP_REMOVE) ||
            !stream->writeInt32(requestId) ||
            !stream->writeWideString(file))
        {
            cerr << "Could not write the remove file request to the stream" << endl;
            return OperationError;
        }
        return OperationPending;
    }

    /* 
     *
     */
    sftp_operation::OperationStatus RemoveFiles::handle_reply(const sftp_hdr & hdr, ssh::IStreamIO * stream)
    {
        if(hdr.type != SSH_FXP_STATUS) {
            cerr << "Didn't get the expected SSH_FXP_STATUS message" << endl;
            return OperationError;
        }
        StatusMessage status;
        if(!status.read(stream)) {
            cerr << "Could not read the status message from the stream" << endl;
            return OperationError;
        }

        switch(status.code)
        {
        case SSH_FX_OK:
            // The file was removed.
            // notify the client
            if(m_notify) m_notify->OnRemoveSuccess(file.c_str());
            if(m_files.empty()) return OperationSuccess;
            else return OperationPending;
        case SSH_FX_NO_SUCH_FILE:
            // notify the client
            if(m_notify) m_notify->OnRemoveFailure(file.c_str(), L"No such file");
            if(m_files.empty()) return OperationFailure;
            else return OperationPending;
        case SSH_FX_PERMISSION_DENIED:
            // notify the client
            if(m_notify) m_notify->OnRemoveFailure(file.c_str(), L"Permission denied");
            if(m_files.empty()) return OperationFailure;
            else return OperationPending;
        default:
            cerr << "Unexpected error code returned" << endl;
            return OperationError;
        }
    }
};