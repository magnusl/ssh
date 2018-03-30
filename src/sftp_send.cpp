#include <iostream>
#include "sftp_send.h"
#include <assert.h>
#include "definitions.h"
#include "ArrayStream.h"
#include "StatusMessage.h"
#include "common.h"
#include "CreateFileRequest.h"
#include "CloseRequest.h"

using namespace std;

namespace sftp
{
    /* Function:        sftp_send::sftp_send
     * Description:     Constructor, performs the required initalization.
     */
    sftp_send::sftp_send(const std::vector<FileTransferElement> files, sftp_notify * notify, sftp_impl * impl) 
        :  sftp_transfer(files, notify, impl)
    {
        m_request   = NULL;
        fileIt      = 0;
        m_state     = SendInit;
        m_attribs   = NULL;
    }

    /* Function:        sftp_send::~sftp_send()
     * Description:     Performs the required cleanup.
     */
    sftp_send::~sftp_send()
    {
        SAFE_DELETE(m_attribs)
        SAFE_DELETE(m_request)
    }

    /* Function:        sftp_send::write
     * Description:     Writes the requests to the stream.
     */
    sftp_operation::OperationStatus sftp_send::write(ssh::ArrayStream * stream, uint32 id)
    {
        // get the current file.
        const FileTransferElement & currentFile = m_files[fileIt];
        switch(m_state)
        {
        case SendInit:
            // Open the file and begin the send process. Windows specific for now
            // first get the file attributes
            if(m_attribs == NULL) m_attribs = m_impl->createAttributeInstance();
            if(m_attribs->setAttributes(currentFile.localFile.c_str()) != STATUS_SUCCESS) {
                // could not load the attributes
                wcerr << "Could not load the file attributes for the file: " << currentFile.localFile << endl;
                // Notify the client that the transfer failed.
                if(m_notify) m_notify->OnTransferFailure(currentFile.localFile.c_str(),
                        currentFile.remoteFile.c_str(),
                        L"The specified file is missing or not accessable");
                // The operation failed.
                return OperationAborted;
            }

            m_file = _wfopen(currentFile.localFile.c_str(), L"rb");
            if(m_file == NULL) {
                // could not open the file
                wcerr << L"Could not open the file: " << currentFile.localFile << endl;
                // notify the client.
                if(m_notify) m_notify->OnTransferFailure(currentFile.localFile.c_str(),
                    currentFile.remoteFile.c_str(),
                    L"Could not open the file for reading");
                return OperationAborted;
            }   
            else
            {
                bool bOverWrite = false;
                SAFE_DELETE(m_request)
                // The file was opened, so first create the file on the remote side.
                m_request = new CreateFileRequest(currentFile.remoteFile.c_str(), m_attribs, m_handle,m_impl, bOverWrite);
                if(m_request == NULL) {
                    // This is a fatal error.
                    wcerr << L"Failed to allocate the CreateFileRequest instance" << endl;
                    return OperationError;
                }
                // write the request
                if(m_request->write(stream, id) != sftp_request_base::StatusIncomplete) {
                    wcerr << L"Could not write the create file request to the stream" << endl;
                    return OperationError;
                }
                
                // Get the filesize, if possible
                if(!m_attribs->getFileSize(m_fileSize))
                    m_fileSize = 0;

                m_bytesWritten  = 0;

                // Change the current state.
                m_state = (m_fileSize == 0 ? EmptyFile : CreatingFile);
                return OperationPending;
            }
        case FileCreated:
            // The file was created on the remote side, so start sending data.
        case NewPacket:
            {
                uint32 max_packet = m_impl->getPacketLimit();
                uint32 dataSize;
                uint64 left = m_fileSize - m_bytesWritten;
                // How much can I write?
                dataSize = (uint32) min(min(left, (uint64) max_packet),(uint64) stream->getFreeSpace());
                // Write the first part of the request.
                if(!stream->writeByte(SSH_FXP_WRITE) ||
                    !stream->writeInt32(id) ||
                    !m_handle.write(stream) ||
                    !stream->writeInt64(m_bytesWritten) ||
                    !stream->writeInt32(dataSize))  // encode it as a packet
                {
                    wcerr << L"Could not write the write request to the stream" << endl;
                    return OperationError;
                }
                // Write the file contents to the stream.
                if(!stream->writeFile(m_file, dataSize)) {
                    wcerr << L"Could not write the file contents to the stream" << endl;
                    return OperationError;
                }

                // update the number of bytes written.
                m_bytesWritten += dataSize;

                // have we written the entire file?
                if(m_bytesWritten == m_fileSize) {
                    // The file has been sent, wait for the reply
                    m_state = FileWritten;
                    return OperationPending;
                } else {
                    // The packet has been sent, now wait.
                    m_state = PacketWritten;
                    return OperationPending;
                }
            }
        case ClosingFailure:
        case ClosingSuccess:
            // The entire packet has been sent, so close the file.
            SAFE_DELETE(m_request)
            m_request = new (std::nothrow) CloseRequest(&m_handle, m_impl);
            if(!m_request->write(stream, id)) {
                wcerr << L"Could not write the close request to the stream" << endl;
                return OperationError;
            }
            return OperationPending;        // wait for the status message
        default:
            wcerr << L"Unknown send-state" << endl;
            assert(false);
            return OperationError;
        }
    }

    /* Function:        sftp_send::parse_reply  
     * Description:     Parses the reply from the server.
     */
    sftp_operation::OperationStatus sftp_send::handle_reply(const sftp_hdr & hdr, ssh::IStreamIO * stream)
    {
        const FileTransferElement & currentFile = m_files[fileIt];
        int nret;
        switch(m_state)
        {
        case EmptyFile:
        case CreatingFile:
            {
                // Requested for the remote file to be created.
                if(m_request == NULL)
                    return OperationError;
                // let the request parse the reply.
                nret = m_request->parse(hdr, stream);
                if(nret == sftp_request_base::StatusSuccess) {
                    // the file was created, change the state.
                    m_state = (m_state == EmptyFile ? ClosingSuccess : FileCreated);
                    // Set the time
                    m_transferStart = time(NULL);
                    return OperationPending;
                } else if(nret == sftp_request_base::StatusFailure) {
                    // could not create the file.
                    if(m_notify) m_notify->OnTransferFailure(currentFile.localFile.c_str(),
                        currentFile.remoteFile.c_str(), m_request->getErrorString());
                    return OperationFailure;
                } else {
                    return OperationError;
                }
            }
        case FileWritten:
        case PacketWritten:
            {
                // Should only get status messages.
                if(hdr.type != SSH_FXP_STATUS) {
                    wcerr << L"Invalid packet sent by the remote host, expected a SSH_FXP_STATUS message" << endl;
                    return OperationError;
                }
                // Read the status message.
                sftp::StatusMessage status;
                if(!status.read(stream)) {
                    wcerr << L"Could not read the status message from the stream" << endl;
                    return OperationError;
                }   
                if(status.code != SSH_FX_OK) {
                    // something went wrong.
                    if(m_notify) m_notify->OnTransferFailure(currentFile.localFile.c_str(),
                        currentFile.remoteFile.c_str(), status.msg.c_str());
                    return OperationFailure;
                } else {
                    // notify the client that data has been sent
                    if(m_notify) {
                        if(m_notify->OnTransferStatus(currentFile.localFile.c_str(),
                            m_bytesWritten, m_fileSize, (uint32)difftime(time(NULL), m_transferStart)) != 0)
                        {
                            // The user aborted the file transfer
                            m_state = ClosingFailure;
                            return OperationPending;
                        }
                    }
                    // The write request was successful, change the state
                    m_state = (m_state == FileWritten ? ClosingSuccess : NewPacket);
                    return OperationPending;
                }
            }
        case ClosingSuccess:
        case ClosingFailure:
            {
                // Should only get status messages.
                if(hdr.type != SSH_FXP_STATUS) {
                    wcerr << L"Invalid packet sent by the remote host, expected a SSH_FXP_STATUS message" << endl;
                    return OperationError;
                }
                // Read the status message.
                sftp::StatusMessage status;
                if(!status.read(stream)) {
                    wcerr << L"Could not read the status message from the stream" << endl;
                    return OperationError;
                }   
                if(status.code != SSH_FX_OK) {
                    // something went wrong.
                    if(m_notify) m_notify->OnTransferFailure(currentFile.localFile.c_str(),
                        currentFile.remoteFile.c_str(), status.msg.c_str());
                    return OperationFailure;
                } else {
                    // The close request was successful.
                    if(m_state == ClosingSuccess) {
                        // The transfer was a success
                        if(m_notify) m_notify->OnTransferSuccess(currentFile.localFile.c_str(), currentFile.remoteFile.c_str());
                        if((fileIt + 1) >= m_files.size()) {
                            // All the file has been sent
                            return OperationSuccess;
                        } else {
                            // More files to send
                            ++fileIt;
                            return OperationPending;
                        }
                    } else {
                        // The transfer was a failure
                        if(m_notify) m_notify->OnTransferFailure(it->localFile.c_str(), it->remoteFile.c_str(),
                            L"The transfer was a failure");
                        return OperationFailure;
                    }
                }
            }
        default:
            assert(false);
            return OperationError;
        }
    }
};