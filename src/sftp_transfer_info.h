#ifndef _SFTP_TRANSFER_INFO_H_
#define _SFTP_TRANSFER_INFO_H_

#include <string>

namespace sftp
{
    /* Struct:          sftp_transfer_info
     * Description:     Contains information about a file transfer.
     */
    struct sftp_transfer_info
    {
        std::wstring remotefile, localfile;
        uint64 size, bytes_transfered;
    };
};

#endif