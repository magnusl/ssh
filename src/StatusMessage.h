#ifndef _STATUSMESSAGE_H_
#define _STATUSMESSAGE_H_

#include "types.h"
#include "IStreamIO.h"
#include <iostream>

namespace sftp
{
    /* Class:           StatusMessage
     * Description:     Represents a status message.
     */
    class StatusMessage
    {
    public:
        /* Function:        read
         * Description:     Reads the status message from the stream
         */
        bool read(ssh::IStreamIO * stream)
        {
            if(!stream->readInt32(code) || 
                !stream->readWideString(msg, SSH_ENCODING_UTF_8) ||
                !stream->readString(language))
            {
                std::cerr << "Could not read the status message from the stream" << std::endl;
                return false;
            }
            return true;
        }
        uint32 code;            // the status code
        std::wstring msg;       // the message
        std::string language;   // language string.
    };
};

#endif