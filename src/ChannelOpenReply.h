#ifndef _CHANNELOPENREPLY_H_
#define _CHANNELOPENREPLY_H_

#include "RequestBase.h"
#include "sshmessages.h"
#include <iostream>

namespace ssh
{
    /* Class:           ChannelOpenReply
     * Description:     Reply message used to indicate the status of a request to open a channel.
     */
    class ChannelOpenReply : public RequestBase
    {
    public:
        enum ReplyType {OpenConfirmation = 0, OpenFailure = 1};

        // SSH_MSG_CHANNEL_OPEN_CONFIRMATION
        ChannelOpenReply(uint32 id, uint32 senderId, uint32 initialWin, uint32 maxPacket)
        {
            m_local         = id;
            m_remote        = senderId;
            m_initialWin    = initialWin;
            m_maxPacket     = maxPacket;
            m_type          = OpenConfirmation;
        }

        // SSH_MSG_CHANNEL_OPEN_FAILURE
        ChannelOpenReply(uint32 id, uint32 reason, const wchar_t * errmsg)
        {
            m_remote        = id;
            m_reason        = reason;
            m_errormsg      = errmsg;
            m_type          = OpenFailure;
        }

        // writes the reply to the stream.
        bool write(IStreamIO * stream)
        {
            switch(m_type)
            {
            case OpenConfirmation:
                if(!stream->writeByte(SSH_MSG_CHANNEL_OPEN_CONFIRMATION) ||
                    !stream->writeInt32(m_remote) ||            //  recipient channel
                    !stream->writeInt32(m_local) ||         // sender channel
                    !stream->writeInt32(m_initialWin) ||        // initial window size
                    !stream->writeInt32(m_maxPacket))           // maximum packet size
                {
                    std::cerr << "ChannelOpenReply::write: failed to write reply to stream" << std::endl;
                    return false;
                }
                break;
            case OpenFailure:
                if(!stream->writeInt32(m_remote) ||             // recipient channel
                    !stream->writeInt32(m_reason) ||            // reason code
                    !stream->readWideString(m_errormsg) ||      // description in ISO-10646 UTF-8 encoding [RFC3629]
                    !stream->writeInt32(0))                     // language tag (empty string)
                {
                    std::cerr << "ChannelOpenReply::write: failed to write reply to stream" << std::endl;
                    return false;
                }
            }
            return true;
        }
    private:
        uint32 m_local, m_remote, m_initialWin, m_maxPacket, m_reason;
        std::wstring m_errormsg;
        ReplyType m_type;
    };
};

#endif