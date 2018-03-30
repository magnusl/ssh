#ifndef _SFTP_NOTIFY_H_
#define _SFTP_NOTIFY_H_

#include "types.h"
#include "sftp_directory.h"
#include <ctime>

namespace sftp
{
    /* Class:           sftp_notify
     * Description:     
     */
    class sftp_notify
    {
    public:
        virtual void OnTransferSuccess(const wchar_t *, const wchar_t *) {}
        virtual void OnTransferFailure(const wchar_t *, const wchar_t *, const wchar_t *) {}
        virtual void OnDirectoryListingSuccess(const sftp_directory *){}
        virtual void OnDirectoryListingFailure(const sftp_directory *){}
        virtual void OnInitalized() {}
        virtual void OnRemoveSuccess(const wchar_t * ) {}
        virtual void OnRemoveFailure(const wchar_t * , const wchar_t *) {}
        virtual int OnTransferStatus(const wchar_t *, uint64, uint64,uint32) {return 0;}
    };
};

#endif