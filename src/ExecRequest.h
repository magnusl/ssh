#ifndef _EXECREQUEST_H_
#define _EXECREQUEST_H_

#include "ChannelRequest.h"
#include <string>
#include "sshmessages.h"
#include <iostream>

namespace ssh
{
    /* Class:           ExecRequest
     * Description:     Used to request the remote execution of a command.
     */
    class ExecRequest : public ChannelRequest
    {
    public:
        ExecRequest(const wchar_t * cmd, uint32 local, uint32 remote) 
            : ChannelRequest(local, remote), m_cmd(cmd) {
        }

        // Writes the request to the stream
        bool write(IStreamIO * stream)
        {
            // Write the data to the stream
            if(!stream->writeByte(SSH_MSG_CHANNEL_REQUEST) ||
                !stream->writeInt32(m_remote) ||
                !stream->writeString("exec") ||
                !stream->writeByte(1) ||
                !stream->writeWideString(m_cmd))
            {
                std::cerr << "Could not write the exec request to the stream" << std::endl;
                return false;
            }
            return true;
        }
    protected:
        std::wstring m_cmd;
    };
};

#endif