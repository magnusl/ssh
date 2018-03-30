#include "sftp_object.h"
#include "ArrayStream.h"
#include "definitions.h"
#include "sftp_messages.h"
#include <iostream>
#include "common.h"
#include <assert.h>
#include "ImplVersion3.h"
#include "ImplVersion4.h"
#include "sftp_directory_listing.h"
#include "RemoveFiles.h"
#include "ssh_channel.h"
#include "FileTransferElement.h"
#include "sftp_send.h"
#include "sftp_read.h"

using namespace std;

namespace sftp
{
    /* Function:        sftp_object
     * Description:     Constructor, performs the required initalization.
     */
    sftp_object::sftp_object(sftp_notify * notify, ssh::Event & ev) : m_notify(notify) , m_startEvent(ev), m_quitEvent(L"")
    {
        m_state = StateWait;
        m_version = 3;  // use 3 for now.
        dst_buff = new (std::nothrow) unsigned char[SFTP_PACKET_LIMIT];
        bytesRead = 0;
    }


    /* Function:        ~sftp_object 
     * Description:     Destructor, perform the required cleanup.
     */
    sftp_object::~sftp_object()
    {
        delete impl;            // delete the implementation
        delete [] dst_buff;     // delete the buffer.
    }

    /* Function:        sftp_object::shutdown
     * Description:     Signals that the connection should be closed.
     */
    void sftp_object::shutdown()
    {
        // Signal it.
        m_quitEvent.signal();
    }

    /* Returns the current status of the object
     */
    ssh::ChannelObject::ObjectStatus sftp_object::status()
    {
        if(m_state == StateFinished || m_quitEvent.isSignaled()) {
            return StatusFinished;
        }   
        else if(m_state == StateWait) {
            if(m_startEvent.isSignaled()) {
                m_state = StateNotInitalized;
                return StatusReady;
            } else {
                return StatusIdle;
            }
        } else {
            switch(m_state)
            {
            case StateWaitInit:
                return StatusIdle;
            case StateInitalized:
                // Initalized, if there is a operation in the queue indicate that we are
                // ready to write som data.
                return (m_queue.empty() ? StatusIdle : StatusReady);
            case StateNotInitalized:
                // Not initalized, so initalize 
                return StatusReady;
            default:
                return StatusError;
            }
        }
    }

    /* Function:        sftp_object::write
     * Description:     Writes data to the stream.
     */
    int sftp_object::write(unsigned char * dst, uint32 size, uint32 & written)
    {
        assert(dst != NULL && size > 0);
        // Skip the first 4 bytes, need room to update the size field.
        ssh::ArrayStream as(dst + sizeof(uint32), size-sizeof(uint32));
        // Use UTF-8 in version 4+
        if(m_version >= SFTP_VERSION_FOUR) as.SetEncoding(SSH_ENCODING_UTF_8);
        switch(m_state)
        {
        case StateNotInitalized:
            {
                // Not initalized, so write the init packet
                if(!as.writeByte(SSH_FXP_INIT) || !as.writeInt32(m_version)) {
                    cerr << "Could not write the initalization packet to the stream" << endl;
                    return FATAL_ERROR;
                }
                // Now write all the extension pairs.
                for(std::list<std::pair<std::string,std::string> >::iterator it = m_extensions.begin();
                    it != m_extensions.end();
                    it++)
                {
                    if(!as.writeString(it->first) || !as.writeString(it->second)) {
                        cerr << "Could not write the extensions to the stream" << endl;
                        return FATAL_ERROR;
                    }
                }
                m_state = StateWaitInit;
                break;
            }
        case StateInitalized:
            if(!m_queue.empty()) {
                // The queue is not empty, so get the first operation from the queue
                sftp_operation * op = m_queue.pop();
                // Get the next request id.
                uint32 id = m_requestId.update();
                // write the request.

                sftp_operation::OperationStatus status = op->write(&as, id);
                if(status == sftp_operation::OperationSuccess || status == sftp_operation::OperationFailure) {
                    // The operation is complete, so delete it.
                    delete op;
                } else if(status == sftp_operation::OperationError) {
                    // A fatal error
                    delete op;
                    return FATAL_ERROR;
                } else if(status == sftp_operation::OperationPending) {
                    // bind the request id identifier with the operation.
                    m_ops[id] = op;
                } else if(status == sftp_operation::OperationNewPacket) {
                    // bind it to both.
                    m_ops[id] = op;
                    // add it to the list again.
                    m_queue.insert(op);
                } else if(status == sftp_operation::OperationWorkingOnPacket) {
                    // just add it to the queue.
                    m_queue.insert(op);
                } else if(status == sftp_operation::OperationAborted) {
                    // The operation was aborted.
                    return STATUS_ACTION_ABORTED;
                } else {
                }
                break;
            } else {
                assert(false);
                return FATAL_ERROR;
            }
        default:
            return STATUS_SUCCESS;
        }

        // Set the correct size
        *((uint32 *)dst) = __htonl32(as.usage());
        // the number of bytes written
        written = as.usage() + sizeof(uint32);
        return true;
    }
    
    /* Function:        sftp_object::parse
     * Description:     Parses the channel data.
     */
    int sftp_object::parse(unsigned char * src, uint32 count)
    {   
        int toRead;
        unsigned char * ptr = src;  // set the index pointer
        uint32 bytesLeft = count;
        while(bytesLeft > 0)
        {
            // First check if we have read the packet size
            if(bytesRead < sizeof(uint32)) {
                toRead =  min(bytesLeft, sizeof(uint32));
                memcpy(dst_buff, ptr, toRead);
                bytesRead += toRead;
                // Update the pointer.
                ptr += toRead;
                bytesLeft -= toRead;
                if(bytesLeft == 0) return STATUS_SUCCESS;
            }
            if(bytesRead == sizeof(uint32))
            {
                // convert the packetSize 
                packetSize = __ntohl32(*((uint32 *)dst_buff));
                if(packetSize > SFTP_PACKET_LIMIT) {
                    cerr << "sftp_object::parse -> SFTP packet to big (" << packetSize <<")" << endl;
                    return FATAL_ERROR;
                }
                // have read the packet size, is it possible to read the entire packet now?
                if(packetSize <= bytesLeft) {
                    ssh::ArrayStream as(ptr, packetSize);
                    if(m_version >= SFTP_VERSION_FOUR) as.SetEncoding(SSH_ENCODING_UTF_8);
                    if(parsePacket(as) != STATUS_SUCCESS)
                        return FATAL_ERROR;
                    bytesLeft -= packetSize;
                    bytesRead = 0;
                    if(bytesLeft == 0) 
                        return STATUS_SUCCESS;
                    ptr += packetSize;
                } else {
                    memcpy(dst_buff + bytesRead, ptr, bytesLeft);
                    bytesRead += bytesLeft;
                    return STATUS_SUCCESS;
                }
            } else {
                // currently working on a packet.
                toRead = min(bytesLeft, packetSize - bytesRead + sizeof(uint32));
                memcpy(dst_buff + bytesRead, ptr, toRead);
                ptr += toRead;
                bytesRead += toRead;
                bytesLeft -= toRead;
                if(bytesRead == (packetSize + sizeof(uint32))) {
                    // read the entire packet
                    ssh::ArrayStream as(dst_buff + sizeof(uint32), packetSize);
                    if(m_version >= SFTP_VERSION_FOUR) as.SetEncoding(SSH_ENCODING_UTF_8);
                    if(parsePacket(as) != STATUS_SUCCESS)
                        return FATAL_ERROR;
                    bytesRead = 0;
                } 
            }
        }
        return STATUS_SUCCESS;
    }

    /* Function:        sftp_object::parsePacket
     * Description:     Parses a packet.
     */
    int sftp_object::parsePacket(ssh::ArrayStream & stream)
    {
        sftp_hdr hdr;
        // Read the message type
        if(!stream.readByte(hdr.type)) {
            cerr << "sftp_object::parsePacket: could not read the packet type from the message" << endl;
            return FATAL_ERROR;
        }
        if(hdr.type == SSH_FXP_VERSION) { /* Handle a version packet */
            if(m_state != StateWaitInit) {
                cerr << "sftp_object::parsePacket: unexpected SSH_FXP_VERSION packet, SFTP already initalized" << endl;
                return FATAL_ERROR;
            }
            // initalization message
            if(!stream.readInt32(this->m_version)) {
                cerr << "sftp_object::parsePacket: Could not read the SFTP version from the message" << endl;
                return FATAL_ERROR;
            }
            if(m_version == SFTP_VERSION_THREE) impl = new (std::nothrow) ImplVersion3;
            else if(m_version == SFTP_VERSION_FOUR) impl = new (std::nothrow) ImplVersion4;
            else {
                cerr << "sftp_object::parsePacket: The SFTP version '" << m_version << "' is not supported" << std::endl;
                return FATAL_ERROR;
            }
            if(impl == NULL) {
                cerr << "sftp_object::parsePacket: allocation failed" << endl;
                return FATAL_ERROR;
            }
            // notify
            if(m_notify) m_notify->OnInitalized();
            // change the state.
            m_state = StateInitalized;
            return STATUS_SUCCESS;
        } else
        {
            /* All other packets begin with the request id */
            if(!stream.readInt32(hdr.request_id)) {
                cerr << "sftp_object::parsePacket: Could not read the request id from the packet" << endl;
                return FATAL_ERROR;
            }
            // Is the request_id valid?
            std::map<uint32, sftp_operation *>::iterator it = m_ops.find(hdr.request_id);
            if(it == m_ops.end()) return FATAL_ERROR;
            sftp_operation * op = it->second;
            // now remove the operation from the map
            m_ops.erase(it);
            /* Let the sftp operation handle the reply */
            sftp_operation::OperationStatus status = op->handle_reply(hdr, &stream);
            // always 
            switch(status)
            {
            case sftp_operation::OperationError:
                return FATAL_ERROR;
            case sftp_operation::OperationFailure:
                // The operation failed, so remove delete it and remove it from the map
                delete op;
                return STATUS_SUCCESS;
            case sftp_operation::OperationSuccess:
                // the operation succeded.
                delete op;
                return STATUS_SUCCESS;
            case sftp_operation::OperationPending:
                // add it to the list
                m_queue.insert(op);
                return STATUS_SUCCESS;
            default:
                assert(false);
                return FATAL_ERROR;
            }
        }
    }

    /* Function:        sftp_object::onClosed
     * Description:     Called when the channel is closed by the remote host
     */
    void sftp_object::onClosed()
    {
        // change the state
        m_state = StateFinished;
        // The client will be notified when the channel is closed.
    }

    /* Function:        sftp_object::onEOF
     * Description:     Called when the remote host sends a EOF message.
     */
    void sftp_object::onEOF()
    {
        m_state = StateFinished;
        // Can't continue with the operations, it's possible to send data, but won't
        // get any feedback. The client will be notified when the channel is closed.

    }

    /* Function:        sftp_object::dir
     * Description:     Used to perform a directory listing operation.
     */
    bool sftp_object::dir(const wchar_t * path, sftp::sftp_directory * dir)
    {
        // Create the operation.
        sftp_directory_listing * op = new sftp_directory_listing(path, dir, m_notify, impl);
        if(op == NULL) {
            cerr << "Could not create the directory listing operation" << endl;
            return false;
        }
        // Add it to the queue. Must be a synchronized operation.
        m_queue.insert(op);
        return true;
    }

    /* Function:        sftp_object::remove
     * Description:     Removes one or more files.
     */
    bool sftp_object::remove(const std::list<std::wstring> & files)
    {
        RemoveFiles * op = new RemoveFiles(files, m_notify, impl);
        if(op == NULL) {
            cerr << "Could not create the remove operation" << endl;
            return false;
        }
        m_queue.insert(op);
        return true;
    }

    /* Function:        SendFiles
     * Description:     Sends files to the remote host.
     */
    bool sftp_object::SendFiles(const wchar_t * local[], const wchar_t * remote[], uint32 number, uint32 mode)
    {   
        std::vector<FileTransferElement> files;
        for(uint32 i = 0;i<number;i++) {
            FileTransferElement element;
            element.localFile = local[i];
            element.remoteFile = remote[i];
            files.push_back(element);
        }

        // Now create the operation
        sftp_send * op = new sftp_send(files, m_notify, impl);
        if(op == NULL) {
            cerr << "Could not create the send operation" << endl;
            return false;
        }
        m_queue.insert(op);
        return true;
    }

    /* Function:        sftp_object::ReadFiles
     * Description:     Reads files from the remote host.
     */
    bool sftp_object::ReadFiles(const wchar_t * local[], const wchar_t * remote[], uint32 number)
    {
        std::vector<FileTransferElement> files;
        for(uint32 i = 0;i<number;i++) {
            FileTransferElement element;
            element.localFile = local[i];
            element.remoteFile = remote[i];
            files.push_back(element);
        }
        sftp_read * op = new (std::nothrow) sftp_read(files, m_notify, impl);
        if(op == NULL) {
            cerr << "Could not create the read operation" << endl;
            return false;
        }
        m_queue.insert(op);
        return true;
    }

};