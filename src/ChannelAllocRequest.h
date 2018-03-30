#ifndef _CHANNELALLOCREQUEST_H_
#define _CHANNELALLOCREQUEST_H_

#include "RequestBase.h"
#include "types.h"

namespace ssh
{
    /* Class:           ChannelAllocRequest
     * Description:     Used to allicate a request.
     */
    class ChannelAllocRequest : public RequestBase
    {
    public:
        ChannelAllocRequest(uint32 localid, const char * type , uint32 window_size, uint32 max_packet)
            : m_type(type), m_id(localid), m_winsize(window_size), m_maxsize(max_packet) {
        }

        // Writes the request to the stream
        bool write(IStreamIO * stream)
        {
            if(!stream->writeByte(SSH_MSG_CHANNEL_OPEN) ||
                !stream->writeString(m_type) ||
                !stream->writeInt32(m_id) ||
                !stream->writeInt32(m_winsize) ||
                !stream->writeInt32(m_maxsize))
            {
                std::cerr << "Could not write the channel alloc request to the stream" << std::endl;
                return false;
            }
            return true;
        }

    private:
        std::string m_type;
        uint32 m_id, m_winsize, m_maxsize;
    };
};

#endif