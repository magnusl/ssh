#ifndef _CHANNELCLOSE_H_
#define _CHANNELCLOSE_H_

#include "RequestBase.h"

namespace ssh
{
    /* Class:           ChannelClose
     * Description:     Used to close a channel.
     */
    class ChannelClose : public RequestBase
    {
    public:
        // constructor.
        ChannelClose(uint32 id) : m_id(id) {
        }

        // writes the request to a stream
        bool write(IStreamIO * stream)
        {
            if(!stream->writeByte(SSH_MSG_CHANNEL_CLOSE) ||
                !stream->writeInt32(m_id))
            {
                return false;
            }
            return true;
        }
    protected:
        uint32 m_id;

    };
};

#endif