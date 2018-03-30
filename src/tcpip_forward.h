#ifndef _TCPIP_FORWARD_H_
#define _TCPIP_FORWARD_H_

#include "GlobalRequest.h"
#include "IStreamIO.h"
#include <iostream>

namespace ssh
{
    /* Class:           tcpip_forward
     * Description:     Used to request a remote tcp-ip port forwarding.
     */
    class tcpip_forward : public GlobalRequest
    {
    public:
        // constructor.
        tcpip_forward(const char * addr, 
            uint32 port, 
            const char * host, 
            uint32 hport, 
            ssh_connection * ssh) : GlobalRequest(ssh)
        {
            m_addr = addr;
            m_port = port;
            m_host = host;
            m_hport = hport;
        }

        /* Function:        write
         * Description:     Writes the request to the stream.
         */
        bool write(IStreamIO * stream);
        // called if the request is successful.
        void success();
        // calld if the request fails.
        void failure();
    protected:
        std::string     m_addr, m_host;
        uint32          m_port, m_hport;
    };
};

#endif