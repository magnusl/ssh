#ifndef _SFTP_DIRECTORY_LISTING_H_
#define _SFTP_DIRECTORY_LISTING_H_

#include "sftp_operation.h"
#include "sftp_directory.h"
#include "sftp_handle.h"
#include "sftp_request_base.h"

namespace sftp
{
    /* Class:           sftp_directory_listing
     * Description:     Used to perform a directory listing.
     */
    class sftp_directory_listing : public sftp_operation
    {
    public:
        sftp_directory_listing(const wchar_t *, sftp_directory *, sftp_notify *, sftp_impl *);
        ~sftp_directory_listing();
        // Handles the reply for the operation.
        OperationStatus handle_reply(const sftp_hdr & hdr, ssh::IStreamIO *);
        // writes a message to the buffer.
        OperationStatus write(ssh::ArrayStream *, uint32);

    protected:

        enum state {InitState = 0, OpenDirRequestSent, DirOpen, PerformingListing, CloseDirectory};

        state               m_state;        // the current state.
        sftp_directory *    m_directory;    // the directory
        std::wstring        m_path;         // the path
        sftp_request_base * m_request;      // the current request.
        sftp_handle         m_handle;       // the directory handle.
    };
};

#endif