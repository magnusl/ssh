#include "sftp_read.h"
#include "common.h"
#include "StatusMessage.h"
#include <iostream>
#include "FileStatRequest.h"
#include "sftp_errorcodes.h"
#include "OpenFileRequest.h"
#include "sftp_impl.h"
#include "CloseRequest.h"

#define MAX_ALLOWED_TRANSFER        1024*1024*1024*50

using namespace std;

namespace sftp
{
    /* Function:        sftp_read::sftp_read
     * Description:     Constructor
     */
    sftp_read::sftp_read(const vector<FileTransferElement> files,       // the files to read
        sftp::sftp_notify *notify,                                      // who to notify            
        sftp::sftp_impl * impl)                                         // the implementation
        : sftp_transfer(files, notify,impl), m_file(NULL)
    {
        fileIt = 0;
        m_state = ReadInit;
        m_request = NULL;
        m_bytesRead = 0;
        m_attrs = NULL;
    }

    /* Function:        sftp_read::~sftp_read
     * Description:     Performs the required cleanup.
     */
    sftp_read::~sftp_read()
    {
        // close the file
        if(m_file != NULL) fclose(m_file);
        SAFE_DELETE(m_request);
        SAFE_DELETE(m_attrs);
    }
    /* Function:        sftp_read::write
     * Description:     Writes data to the stream.
     */
    sftp_operation::OperationStatus sftp_read::write(ssh::ArrayStream * stream ,uint32 id)
    {
        const FileTransferElement & currentFile = m_files[fileIt];
        switch(m_state)
        {
        case ReadInit:
            {
                // create the local file, but make sure it dosen't already exist
                m_file = _wfopen(currentFile.localFile.c_str(), L"r");
                if(m_file != NULL) {
                    // the file already exists
                    fclose(m_file);
                    if(m_notify) m_notify->OnTransferFailure(currentFile.localFile.c_str(),
                        currentFile.remoteFile.c_str(), L"The local file already exists, aborting");
                    //cerr << "The local file already exists" << endl;
                    return OperationAborted;
                }
                // the file does not exist, so create it.
                m_file = _wfopen(currentFile.localFile.c_str(), L"wb");
                if(m_file == NULL) {
                    m_notify->OnTransferFailure(currentFile.localFile.c_str(),
                        currentFile.remoteFile.c_str(), L"Could not create the file on the local host");
                    return OperationAborted;
                }
                // Open the file on the remote host
                SAFE_DELETE(m_request);
                m_request = new (std::nothrow) OpenFileRequest(currentFile.remoteFile.c_str(), &m_handle, m_impl);
                if(m_request == NULL) {
                    cerr << "Could not create the OpenFileRequest" << endl;
                    return OperationError;
                }
                if(m_request->write(stream, id) != sftp_request_base::StatusIncomplete) {
                    cerr << "Could not write the OpenFileRequest to the stream" << endl;
                    return OperationError;
                }
                // Now wait until the file is opened.
                m_bytesRead = 0;
                m_state = OpeningFile;
                return OperationPending;
            }
        case FileOpened:
            {
                if(m_attrs == NULL) m_attrs = m_impl->createAttributeInstance();
                // The file was opened, now get the stats.
                SAFE_DELETE(m_request);
                m_request = new (std::nothrow) FileStatRequest(&m_handle, m_attrs, m_impl);
                if(m_request == NULL) {
                    cerr << "Could not create the FileStatRequest" << endl;
                    return OperationError;
                }
                if(m_request->write(stream, id) != sftp_request_base::StatusIncomplete) {
                    cerr << "Could not write the FileStatRequest to the stream" << endl;
                    return OperationError;
                }
                m_state = GettingFileStat;
                return OperationPending;
            }
        case GotFileStat:
            {
                // got the file stat, begin the actual transfer.
                if(!m_attrs->getFileSize(m_fileSize))
                    m_fileSize = 0;

                if(m_fileSize == 0) {
                    // empty file, so close it on the remote side
                    SAFE_DELETE(m_request);
                    m_request = new (std::nothrow) CloseRequest(&m_handle, m_impl);
                    if(m_request == NULL) {
                        cerr << "Could not create the close request" << endl;
                        return OperationError;
                    }
                    // write the close request
                    if(!m_request->write(stream, id)) {
                        cerr << "Could not write the close request to the stream" << endl;
                        return OperationError;
                    }
                    // change the state
                    m_state = CloseSuccess;
                    return OperationPending;
                } 
                m_transferStart = time(NULL);
                // else fall into ReadingPacket
                m_state = ReadingPacket;
            }
        case ReadingPacket:
            {
                // how much should I read?
                uint32 limit = m_impl->getPacketLimit(),num;
                uint64 left = (m_fileSize - m_bytesRead);
                num = static_cast<uint32>(left > static_cast<uint64>(limit) ? limit : static_cast<uint32>(left));
                // Now write the request.
                if(!stream->writeByte(SSH_FXP_READ) ||
                    !stream->writeInt32(id) ||
                    !m_handle.write(stream) ||
                    !stream->writeInt64(m_bytesRead) ||
                    !stream->writeInt32(num)) 
                {
                    cerr << "Could not write the read request to the stream" << endl;
                    return OperationError;
                }
                return OperationPending;
            }
        case CloseSuccess:
        case CloseFailure:
            {
                // Close the file
                SAFE_DELETE(m_request);
                m_request = new (std::nothrow) CloseRequest(&m_handle, m_impl);
                if(m_request == NULL) {
                    cerr << "Could not create the close request" << endl;
                    return OperationError;
                }
                // Write the request.
                if(!m_request->write(stream, id)) {
                    cerr << "Could not write the close request to the stream" << endl;
                    return OperationError;
                }
                return OperationPending;
            }
        default:
            {
                cerr << "Unknown read state, aborting" << endl;
                assert(false);
                return OperationError;
            }
        }
    }

    /* Function:        sftp_read::handle_reply
     * Description:     Handles the servers responses.
     */
    sftp_operation::OperationStatus sftp_read::handle_reply(const sftp_hdr & hdr, ssh::IStreamIO * stream)
    {
        const FileTransferElement & currentFile = m_files[fileIt];
        switch(m_state)
        {
        case OpeningFile:
            {
                sftp_request_base::Status ret = m_request->parse(hdr, stream);
                if(ret == sftp_request_base::StatusSuccess) {
                    // the file was opened.
                    m_state = FileOpened;
                    return OperationPending;
                } else if(ret == sftp_request_base::StatusFailure) {
                    // could not open the file, close the local file
                    fclose(m_file); m_file = NULL;
                    // notify the client.
                    if(m_notify) m_notify->OnTransferFailure(currentFile.localFile.c_str(),
                        currentFile.remoteFile.c_str(),m_request->getErrorString());
                    return OperationFailure;
                } else {
                    // close the file
                    fclose(m_file); m_file = NULL;
                    return OperationError;
                }
            }
        case GettingFileStat:
            {
                sftp_request_base::Status ret = m_request->parse(hdr, stream);
                if(ret == sftp_request_base::StatusSuccess) {
                    // we got the file stats.
                    m_state = GotFileStat;
                    return OperationPending;
                } else if(ret == sftp_request_base::StatusFailure) {
                    // the stat request failed, close the local file
                    fclose(m_file); m_file = NULL;
                    // now close the remote file
                    m_state = CloseFailure;
                    return OperationPending;
                } else {
                    return OperationError;
                }
            }
        case ReadingPacket:
            {
                if(hdr.type == SSH_FXP_DATA)
                {
                    // Data was read
                    uint32 length;
                    if(!stream->readInt32(length)) {
                        cerr << "Could not read the length field of the SSH_FXP_DATA packet" << endl;
                        return OperationError;
                    }
                    // now read the data to the file
                    if(!stream->readFile(m_file, (int) length)) {
                        cerr << "Could not write the file contents to the local file" << endl;
                        return OperationError;
                    }
                    m_bytesRead += length;
                    // Should we notify the client?
                    if(m_notify)
                    {
                        if(m_notify->OnTransferStatus(currentFile.localFile.c_str(),
                            m_bytesRead, m_fileSize, (uint32)difftime(time(NULL), m_transferStart)) != 0)
                        {
                            // The client aborted the transfer
                            m_state = CloseFailure;
                            return OperationPending;
                        }
                    }
                    if(m_bytesRead >= m_fileSize) {
                        // read the entire file
                        fclose(m_file); m_file = NULL;
                        m_state = CloseSuccess;
                        return OperationPending;
                    }
                    else {
                        // more data to read
                        return OperationPending;
                    }
                }
                else if(hdr.type == SSH_FXP_STATUS) {
                    // read the status message
                    StatusMessage status;
                    if(!status.read(stream)) {
                        cerr << "Could not read the status message from the stream" << endl;
                        return OperationError;
                    }
                    switch(status.code)
                    {
                    case SSH_FX_EOF:
                        // The entire file has been read.
                        fclose(m_file); m_file = NULL;
                        m_state = CloseSuccess;
                        return OperationPending;
                    case SSH_FX_FAILURE:
                        // could not perform the read
                        m_state = CloseFailure;
                        return OperationPending;
                    default:
                        assert(false);
                        return OperationError;
                    }
                } else {
                    cerr << "Unknown reply to the SSH_FXP_READ messae" << endl;
                    return OperationError;
                }
            }
        case CloseSuccess:
        case CloseFailure:
            {
                if(hdr.type != SSH_FXP_STATUS)  {
                    cerr << "Unknown reply to the close request, expected a SSH_FXP_STATUS message" << endl;
                    return OperationError;
                }
                // Now read the status message
                StatusMessage status;
                if(!status.read(stream)) {
                    cerr << "Could not read the status message" << endl;
                    return OperationError;
                }
                // now parse the status message
                switch(status.code)
                {
                case SSH_FX_FAILURE:
                case SSH_FX_OK:
                    {
                        // The file was closed
                        if(m_state == CloseSuccess) {
                            if(m_notify) m_notify->OnTransferSuccess(currentFile.localFile.c_str(),
                                currentFile.remoteFile.c_str());
                            // Have we read all the files?
                            if((fileIt + 1) >= m_files.size()) {
                                return OperationSuccess;
                            } else {
                                ++fileIt;
                                return OperationPending;
                            }
                        } else {
                            if(m_notify) m_notify->OnTransferFailure(currentFile.localFile.c_str(),
                                currentFile.remoteFile.c_str(), L"The transfer was a failure");
                            return OperationFailure;
                        }
                    }
                default:
                    cerr << "A error while closing the file on the remote host, what should we do?" << endl;
                    return OperationError;
                }
            }
        default:
            {
                cerr << "Unknown read state, aborting" << endl;
                assert(false);
                return OperationError;
            }
        }
    }
};