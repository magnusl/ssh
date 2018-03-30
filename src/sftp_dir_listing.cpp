#include "sftp_dir_listing.h"
#include <iostream>

using namespace std;

namespace sftp
{
    /* Function:        sftp_dir_listing::sftp_dir_listing
     * Description:     Used to perform a directory listing.
     */
    sftp_dir_listing::sftp_dir_listing(sftp_connection * connection,sftp_notify * notify, 
        sftp::sftp_directory * dir) 
        : m_con(connection), m_notify(notify), m_dir(dir)
    {
    }

    /* Function:        sftp_dir_listing::perform
     * Description:     Performs/initalizes the directory listing.
     */
    bool sftp_dir_listing::perform(const wchar_t * path)
    {
        m_path = path;
        if(!m_con->open_dir(path, &m_handle, this)) {
            wcerr << L"Could not create the directory listing request" << endl;
            return false;
        }
        return true;
    }

    /* Function:        sftp_dir_listing::OnOpenDirectorySuccess
     * Description:     The directory was opened.
     */
    void sftp_dir_listing::OnOpenDirectorySuccess()
    {
        // the directory was opened. Now perform the actual directory listing.
        if(!m_con->dir(&m_handle, m_dir, this)) {
            if(m_notify) m_notify->OnDirListingFailure(m_path,0);
        }
    }

    /* 
     *
     */
    void sftp_dir_listing::OnOpenDirectoryFailure(uint32 reason)
    {
        // could not open the directory.
    
    }

    /* 
     *
     */
    void sftp_dir_listing::OnDirectoryListing()
    {
        // close the directory. We don't really care if the directory was closed successfully or not. We
        // don't want to wait for the reply of the close message
        if(!m_con->close(&m_handle, this, false)) 
        {
            // handle this later.
        }
    }

    void sftp_dir_listing::OnDirectoryListingFailure(uint32 reason)
    {
        // I don't want to be notified.
        m_con->close(&m_handle, this, false);
        if(m_notify) m_notify->OnDirListingFailure(m_path, reason);

    }


};