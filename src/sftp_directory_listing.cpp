#include "sftp_directory_listing.h"
#include <assert.h>
#include <iostream>
#include "sftp_messages.h"
#include "common.h"
#include "OpenDirRequest.h"
#include "DirListingRequest.h"
#include "CloseRequest.h"

using namespace std;

namespace sftp
{
    /* Function:        sftp_directory_listing::sftp_directory_listing
     * Description:     Constructor, performs the required initalization.
     */
    sftp_directory_listing::sftp_directory_listing(const wchar_t * path,        // the path
                                                sftp_directory * dir,       // the directory (destination)
                                                sftp_notify * notify,       // notification
                                                sftp_impl * impl)           // the SFTP implementation used.
        : sftp_operation(notify,impl), 
        m_path(path), 
        m_directory(dir),
        m_request(NULL)
    {
        // Set the inital state
        m_state = sftp_directory_listing::InitState;
    }   

    sftp_directory_listing::~sftp_directory_listing()
    {
        SAFE_DELETE(m_request);
    }

    /* Function:        sftp_directory_listing::write
     * Description:     Writes the requests to the file.
     */
    sftp_operation::OperationStatus sftp_directory_listing::write(ssh::ArrayStream * stream, uint32 requestId)
    {
        switch(m_state)
        {
        case sftp_directory_listing::InitState:
            // Delete the old request
            SAFE_DELETE(m_request);
            // Must open the directory
            m_request = new OpenDirRequest(m_path.c_str(), &m_handle, m_impl);
            if(m_request == NULL) {
                cerr << "could not create the directory open request" << endl;
                return OperationError;
            }
            // Write the request to the stream
            if(m_request->write(stream, requestId) != sftp_request_base::StatusIncomplete) {
                cerr << "Could not write the request to the stream" << endl;
                return OperationError;
            }
            // Change the state.
            m_state = sftp_directory_listing::OpenDirRequestSent;
            break;
        case sftp_directory_listing::DirOpen:
            // Delete the old request
            SAFE_DELETE(m_request);
            // Now perform the actual directory listing.
            m_request = new (std::nothrow) DirListingRequest(&m_handle, m_directory, m_impl);
            if(m_request == NULL) {
                cerr << "Could not create the directory listing request" << endl;
                return OperationError;
            }
            // Write the request to the stream
            if(m_request->write(stream, requestId) != sftp_request_base::StatusIncomplete) {
                cerr << "Could not write the request to the stream" << endl;
                return OperationError;
            }
            // now change the state
            m_state = sftp_directory_listing::PerformingListing;
            break;
        case sftp_directory_listing::PerformingListing:
            //cerr << "Continuting with the directory listing" << std::endl;
            // Write the request to the stream
            if(m_request->write(stream, requestId) != sftp_request_base::StatusIncomplete) {
                cerr << "Could not write the request to the stream" << endl;
                return OperationError;
            }
            break;
        case sftp_directory_listing::CloseDirectory:
            // Delete the old request
            SAFE_DELETE(m_request);
            // Create the close request
            m_request = new (std::nothrow) CloseRequest(&m_handle, m_impl);
            if(m_request == NULL) {
                // Return a error
                return OperationError;
            }
            // write the request to the stream
            if(m_request->write(stream, requestId) != sftp_request_base::StatusIncomplete) {
                cerr << "Could not write the request to the stream" << endl;
                return OperationError;
            }
            break;
        default:
            // Should never happen.
            assert(false);
            return OperationFailure;
        }
        return OperationPending;
    }

    /* Function:        sftp_directory_listing::parse   
     * Description:     Parses the reply.
     */
    sftp_operation::OperationStatus 
        sftp_directory_listing::handle_reply(const sftp_hdr & hdr, ssh::IStreamIO * stream)
    {
        if(m_request == NULL) return sftp_operation::OperationError;
        sftp_request_base::Status nret;
        if(hdr.type != SSH_FXP_STATUS && hdr.type != SSH_FXP_HANDLE && hdr.type != SSH_FXP_NAME) {
            // Invalid message.
            cerr << "Invalid message: " << (uint32) hdr.type << endl;
            return OperationError;
        } 
        
        nret = m_request->parse(hdr, stream);
        if(nret == sftp_request_base::StatusError) {
            cerr << "A error occurred while parsing the reply" << endl;
            return OperationError;
        }

        switch(m_state)
        {
        case sftp_directory_listing::OpenDirRequestSent:
            // The Open request has been sent.
            if(nret == sftp_request_base::StatusSuccess) {
                // The directory was opened, change the state
                m_state = sftp_directory_listing::DirOpen;
                // continue with the operation.
                return sftp_operation::OperationPending;
            } else if(nret == sftp_request_base::StatusFailure) {
                wcerr << L"Could not open the directory" << endl;
                // Could not open the directory
                m_notify->OnDirectoryListingFailure(m_directory);
                // stop the entire operation.
                return sftp_operation::OperationFailure;
            } else {
                // Should not happen
                assert(false);
                return OperationFailure;
            }
        case sftp_directory_listing::PerformingListing:
            // Performing the directory listing.
            if(nret == sftp_request_base::StatusSuccess) {
                // The request is complete, notify the client and
                // then close the directory.
                m_notify->OnDirectoryListingSuccess(0);
                m_state = sftp_directory_listing::CloseDirectory;
                return sftp_operation::OperationPending;
            } else if(nret == sftp_request_base::StatusFailure) {
                // Could not perform the directory listing, notify the client and then
                // close the directory
                m_state = sftp_directory_listing::CloseDirectory;
                return sftp_operation::OperationPending;
            } else if(nret == sftp_request_base::StatusIncomplete) {
                // waiting for more data.
                return sftp_operation::OperationPending;;
            } else {
                assert(false);
                return OperationFailure;
            }
        case sftp_directory_listing::CloseDirectory:
            return OperationSuccess;
        default:
            assert(false);
            return OperationFailure;
        }
    }
};