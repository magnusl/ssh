#ifndef _CHANNELREQUEST_H_
#define _CHANNELREQUEST_H_

#include "RequestBase.h"
#include "types.h"

namespace ssh
{
    /* Class:           ChannelRequest
     * Description:     Used to perform a channel specific request
     */
    class ChannelRequest : public RequestBase
    {
    public:
        ChannelRequest(uint32 id, uint32 remote) : m_local(id), m_remote(remote) {
        }

        uint32 getLocalId() {return m_local;}
    protected:
        uint32 m_local, m_remote;
    };
};

#endif