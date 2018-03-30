#ifndef _FILETRANSFERELEMENT_H_
#define _FILETRANSFERELEMENT_H_

#include <string>

namespace sftp
{
    /* Struct:          FileTransferElement
     * Description:     Represents a file to send.
     */
    struct FileTransferElement
    {
        std::wstring localFile,         // the local file. src when sending, dst when reading       
                     remoteFile;        // the remote file, dst when sending, src when reading.
    };
};

#endif