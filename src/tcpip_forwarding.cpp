#include "ssh_connection.h"
#include "ForwardingInfo.h"
#include <string>
#include <iostream>
#include "TCPForwardingObject.h"
#include "ChannelOpenReply.h"
#include "tcpip_channel.h"

using namespace std;

namespace ssh
{
    /* Function:        ssh_connection::handle_portforwarding
     * Description:     Handles a remote port forwarding.
     */
    int ssh_connection::handle_portforwarding()
    {
        uint32 remoteId, maxSize, initialWin, port, oport;
        string addr, oaddr;
        if(!transportLayer.readInt32(remoteId) ||
            !transportLayer.readInt32(initialWin) ||
            !transportLayer.readInt32(maxSize) ||
            !transportLayer.readString(addr) ||     // connected addr
            !transportLayer.readInt32(port) ||      // connected port
            !transportLayer.readString(oaddr) ||    // originator addr
            !transportLayer.readInt32(oport))       // originator port
        {
            cerr << "ssh_connection::handle_portforwarding: failed to read from the stream" << endl;
            return STATUS_FAILURE;
        }
        ForwardingInfo info;
        if(!remoteForwarding.find_match(HostCompare(addr, port), info)) {
            // send a response to the server
            ChannelOpenReply * reply = new (std::nothrow) ChannelOpenReply(remoteId, 0, 
                L"Remote portforwarding for the supplied addr/port combination has not been requested");
            if(reply == NULL) {
                cerr << "ssh_connection::handle_portforwarding(): allocation failed" << endl;
                return FATAL_ERROR;
            }
            requests.insert(reply);
            // no match, wasen't requested.
            return STATUS_SUCCESS;
        }
        
        // get the 
        uint32 id = next_channel_id();
        // create the channel.
        tcpip_channel * channel = new (std::nothrow) remote_tcp_forwarding(id,info.remoteAddr, info.remotePort,this);
        // initalize the channel with the remote info.
        channel->initalize(remoteId, initialWin,maxSize, config);
        // add the channel to the list
        m_channelLock.lock();
        m_channels[id] = channel;
        m_channelLock.unlock();
        // return success.
        return STATUS_SUCCESS;
    }
    /* 
     * ssh_connection
     */

    // unregisters a local forwarding
    int ssh_connection::unregister_local_forwarding(const char * localAddr, uint32 localPort)
    {   
        // remove the entry.
        localForwarding.remove(HostCompare(localAddr, localPort));
        return STATUS_SUCCESS;
    }

    // registers a remote forwarding.
    int ssh_connection::register_remote_forwarding(const char * bindaddr, uint32 bindport, const char * host, uint32 port)
    {
        ForwardingInfo info;
        info.localAddr = bindaddr;
        info.localPort = bindport;
        info.remoteAddr = host;
        info.remotePort = port;
        // add the entry to the list.
        remoteForwarding.insert(info);
        return STATUS_SUCCESS;
    }

    // unregisters a remote forwarding.
    int ssh_connection::unregister_remote_forwarding(const char * bindaddr, uint32 bindport)
    {
        remoteForwarding.remove(HostCompare(bindaddr, bindport));
        return STATUS_SUCCESS;
    }

    /* Function:        ssh_connection::register_local_forwarding
     * Description:     Registers a local tcp/ip forwarding.
     */
    int ssh_connection::register_local_forwarding(
        const char * localAddr,     // local address to bind
        uint32 localPort,           // local port to bind
        const char * host,          // remote host
        uint32 hostPort)            // remote port.
    {
        ForwardingInfo info;
        // convert the port to a string, required for the bind call
        stringstream ss; 
        ss << localPort;
        // setup the entry.
        info.localAddr      = localAddr;
        info.remoteAddr     = host;
        info.localPort      = localPort;
        info.remotePort     = hostPort;
        
        // bind the socket.
        int nret = info.sock.bind(localAddr, ss.str().c_str());
        if(nret != STATUS_SUCCESS) {
            DebugPrint("Failed to bind socket");
            return nret;
        }
        // insert the entry in the list.
        localForwarding.insert(info);
        return STATUS_SUCCESS;
    }

    /* Function:        local_tcp_forwarding::local_tcp_forwarding
     *
     */
    local_tcp_forwarding::local_tcp_forwarding(SocketLayer & layer, ssh_connection * ssh, uint32 id) 
        : tcpip_channel(ssh,id,0)
    {
    }

    /* Function:        local_tcp_forwarding::OnChannelSuccess
     * Description:     The tunnel was established correctly.
     */
    void local_tcp_forwarding::OnChannelSuccess()
    {

    }

    void local_tcp_forwarding::OnChannelFailure(uint32 code, const char *reason)
    {
    }

    int local_tcp_forwarding::write(byte *, uint32, uint32 &)
    {
        return 0;
    }

    int local_tcp_forwarding::parse(const byte *dst, unsigned int)
    {
        return 0;
    }

    
    remote_tcp_forwarding::remote_tcp_forwarding(uint32 id, 
        const std::string &addr, 
        uint32 port, 
        ssh::ssh_connection * ssh) : tcpip_channel(ssh, id, SSH_CREATED_BY_SERVER)
    {
    }
    
    /* Function:        remote_tcp_forwarding::update
     * Description:     Updates the channel.
     */
    int remote_tcp_forwarding::update()
    {
        int nret;
        if(!bConnected) {   // try to connect.
            nret = m_sock.connect_nonblock(m_addr.c_str(), m_port);
            if(nret == STATUS_SUCCESS) {            // connected
                bConnected = true;
                // notify the server that the channel was opened correctly.
                ChannelOpenReply * reply = 
                    new (std::nothrow) ChannelOpenReply(m_id, m_senderId, static_cast<uint32>(readbuff.len), 32000);
                if(reply == NULL) {
                    return FATAL_ERROR;
                }
                // change the state to active
                // add the request.
                m_ssh->add_request(reply);
                return STATUS_SUCCESS;
            } else if(nret == STATUS_PENDING) {     // connecting
                return STATUS_SUCCESS;
            } else {    // error.
                ChannelOpenReply * reply = 
                    new (std::nothrow) ChannelOpenReply(m_senderId, SSH_OPEN_CONNECT_FAILED, L"Could not connect to host.");
                if(reply == NULL) return FATAL_ERROR;
                // add the reply
                m_ssh->add_request(reply);
                // remove the channel
                return STATUS_SUCCESS;
            }
        } else {
            // connected.
        }
        return STATUS_SUCCESS;
    }

    bool remote_tcp_forwarding::isDataAvailable()
    {
        return false;
    }

    int remote_tcp_forwarding::UpdateWindow()
    {
        return 0;
    }

    int remote_tcp_forwarding::write(byte *, uint32, uint32 &)
    {
        return 0;
    }

    int remote_tcp_forwarding::parse(const byte *, unsigned int)
    {
        return 0;
    }


};