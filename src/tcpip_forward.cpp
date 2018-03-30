#include "tcpip_forward.h"
#include "ssh_connection.h"
#include "sshmessages.h"
#include "ForwardingInfo.h"
#include "OpenChannelDirectTCP.h"
#include "tcpip_channel.h"

namespace ssh
{
    /* Function:        tcpip_forward::write
     * Description:     Writes the request to the stream
     */
    bool tcpip_forward::write(IStreamIO * stream)
    {
        if(!stream->writeByte(SSH_MSG_GLOBAL_REQUEST) ||
            !stream->writeString("tcpip-forward") ||        // request name
            !stream->writeByte(1) ||                        // want reply
            !stream->writeString(m_addr) ||                 // addr to bind
            !stream->writeInt32(m_port))                    // port to bind.
        {
            std::cerr << "tcpip_forward::write: failed to write the request to the stream" << std::endl;
            return false;
        }
        return true;
    }

    /* Function:        tcpip_forward::success 
     * Description:     Called if the request is successful.
     */
    void tcpip_forward::success()
    {
        m_ssh->register_remote_forwarding(m_addr.c_str(), m_port, m_host.c_str(), m_hport);
    }

    /* Function:        tcpip_forward::failure 
     * Description:     Called if the request fails.
     */
    void tcpip_forward::failure()
    {
    }

    /* Function:        LocalForwardingCheck
     * Description:     Checks for any incoming connections.
     */
    void LocalForwardingCheck::operator() (ForwardingInfo & obj)
    {
        SocketLayer sock;
        std::string addr;
        int port;
        if(obj.sock.accept_connection(sock, addr, port))    // a incoming connection.
        {
            // allocate a new channel instance.
            uint32 id = m_ssh->next_channel_id();
            // create a new channel instance.
            local_tcp_forwarding * channel = new (std::nothrow) local_tcp_forwarding(sock, m_ssh, id);
            // allocate a new request.
            OpenChannelDirectTCP * request = 
                new (std::nothrow) OpenChannelDirectTCP(id, obj.remoteAddr,obj.remotePort,addr,port);
            // add the request to the list
            m_ssh->request_channel(channel, request);
        }
    }

};