#ifndef _SFTP_DIRECTORY_OBSERVER_H_
#define _SFTP_DIRECTORY_OBSERVER_H_

namespace sftp
{
    /* Class:           sftp_directory_observe
     * Description:     Used to notify observers about directory changes.
     */
    class sftp_directory_observer
    {
    public:
        /* Directory Listing */
        virtual void OnDirectoryListing(const wchar_t *, const sftp_directory &) = 0;
    };
};

#endif