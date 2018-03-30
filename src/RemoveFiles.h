#ifndef _REMOVEFILES_H_
#define _REMOVEFILES_H_

#include "sftp_operation.h"
#include <list>

namespace sftp
{
    /* Class:           RemoveFiles
     * Description:     Used to remove one or more files from the server.
     */
    class RemoveFiles : public sftp_operation
    {
    public:
        RemoveFiles(const std::list<std::wstring> & files,      // the files to remove on the remote side
            sftp_notify *, sftp_impl *);
        // Handles the reply for the operation.
        OperationStatus handle_reply(const sftp_hdr & hdr, ssh::IStreamIO *);
        // writes a message to the buffer.
        OperationStatus write(ssh::ArrayStream *, uint32);

    private:
        std::list<std::wstring> m_files;        // the list with the files
        std::wstring file;                      // the current file.
    };
};

#endif