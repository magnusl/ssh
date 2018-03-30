#ifndef _SFTP_RAW_NOTIFY_H_
#define _SFTP_RAW_NOTIFY_H_

#include <string>
#include "types.h"
#include "sftp_handle.h"
#include "sftp_directory.h"

namespace sftp
{
    /* Class:           sftp_raw_notify
     * Description:     Notification about lowlevel sftp requests.
     */
    class sftp_raw_notify
    {
    public:
        virtual void OnOpenFileSuccess(sftp_handle *) {}
        virtual void OnOpenFileFailure(uint32) {}
        virtual void OnOpenDirectorySuccess() {}
        virtual void OnOpenDirectoryFailure(uint32) {}
        virtual void OnReadSuccess(sftp_handle *) {}
        virtual void OnReadFailure(sftp_handle *) {}
        virtual void OnWriteSuccess(uint32) {}
        virtual void OnWriteFailure(uint32) {}
        virtual void OnConnectionEstablished() {}
        virtual void OnClose() {}
        virtual void OnDirectoryListing(const sftp_directory *) {}
        virtual void OnDirectoryListingFailure(uint32) {}
    };
};

#endif