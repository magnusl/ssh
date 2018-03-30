#ifndef _OPENCHANNELDIRECTTCP_H_
#define _OPENCHANNELDIRECTTCP_H_

#include "RequestBase.h"
#include <string>
#include <iostream>

namespace ssh
{
    /* Class:           OpenChannelDirectTCP
     * Description:     Used to request a TCP forwarding.
     */
    class OpenChannelDirectTCP : public RequestBase
    {
    public:
        OpenChannelDirectTCP(uint32 id,         // channel id
            const std::string & host,           // host to connect
            uint32 hport,                       // host port.
            const std::string & addr,           // originator addr
            uint32 port)                        // originator port
            : m_id(id),
            m_host(host),
            m_hport(hport),
            m_addr(addr),
            m_port(port)
        {
        }

        // writes the request to a stream.
        bool write(IStreamIO * stream)
        {
            if(!stream->writeByte(SSH_MSG_CHANNEL_OPEN) ||      // message id.
                !stream->writeString("direct-tcpip") ||         
                !stream->writeInt32(m_id) ||                    // sender channel
                !stream->writeInt32(25000) ||                   // initial window size
                !stream->writeInt32(30000) ||                   // max packet size
                !stream->writeString(m_host) ||
                !stream->writeInt32(m_hport) ||
                !stream->writeString(m_addr) ||
                !stream->writeInt32(m_port))
            {
                std::cerr << "OpenChannelDirectTCP::write(): failed to write request" << std::endl;
                return false;
            }
            return true;
        }
    protected:
        std::string m_addr, m_host;
        uint32 m_hport, m_port, m_id;

    };
};

#endif