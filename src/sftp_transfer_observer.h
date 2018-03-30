#ifndef _SFTP_TRANSFER_OBSERVER_H_
#define _SFTP_TRANSFER_OBSERVER_H_

#include "sftp_transfer_info.h"

namespace sftp
{
    /* Class:           sftp_transfer_observer
     * Description:     Used to notify a observer about the transfers.
     */
    class sftp_transfer_observer
    {
    public:
        // A new file transfer
        virtual void OnNewTransfer(const sftp_transfer_info &)      = 0;
        // A file transfer was finished
        virtual void OnTransferFinished(const sftp_transfer_info &) = 0;
        // A file transfer failed
        virtual void OnTransferError(const sftp_transfer_info &)    = 0;
    };
};

#endif