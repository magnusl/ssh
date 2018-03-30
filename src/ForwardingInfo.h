#ifndef _FORWARDINGINFO_H_
#define _FORWARDINGINFO_H_

#include <string>
#include "types.h"
#include "common.h"

namespace ssh
{
    class ssh_connection;

    /* Struct:      ForwardingInfo
     *
     */
    struct ForwardingInfo
    {
        std::string localAddr, remoteAddr;
        uint32 localPort, remotePort;
        SocketLayer sock;
    };

    /* Struct:          HostCompare
     * Description:     Functor object used to remote elements for a list.
     */
    struct HostCompare
    {
        // constructor
        HostCompare(const std::string & host, uint32 port) 
            : m_host(host), m_port(port)
        {   
        }

        HostCompare(const char * host, uint32 port) 
            : m_host(host), m_port(port) 
        {
        }
    
        // compare function.
        bool operator() (ForwardingInfo & obj) {
            if(obj.localAddr == m_host && obj.localPort == m_port) {
                return true;
            }
            return false;
        }

        std::string m_host;
        uint32 m_port;
    };

    // checks for any incoming connection
    struct LocalForwardingCheck
    {
        LocalForwardingCheck(ssh_connection * ssh) : m_ssh(ssh) {
        }

        void operator() (ForwardingInfo & obj);
        ssh_connection * m_ssh;
    };
};

#endif