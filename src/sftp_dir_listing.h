#ifndef _SFTP_DIR_LISTING_H_
#define _SFTP_DIR_LISTING_H_

#include "sftp.h"
#include "sftp_directory.h"
#include "types.h"
#include "sftp_raw_notify.h"
#include "sftp_notify.h"

namespace sftp
{
    /* Class:           sftp_dir_listing
     * Description:     Performs the directory listing of the specified directory.
     */
    class sftp_dir_listing : public sftp_raw_notify
    {
    public:
        // constructor.
        sftp_dir_listing(sftp_connection *, sftp_notify *, sftp_directory *);
        // performs the directory listing.
        bool perform(const wchar_t *);
        void OnOpenDirectorySuccess();
        void OnOpenDirectoryFailure(uint32);
        void OnDirectoryListing();
        void OnDirectoryListingFailure(uint32);
    protected:
        sftp_notify * m_notify;
        sftp_handle m_handle;
        sftp_connection * m_con;
        sftp_directory * m_dir;
        std::wstring m_path;
    };
};


#endif