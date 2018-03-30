#ifndef _TCPIP_CHANNEL_H_
#define _TCPIP_CHANNEL_H_

#include "ssh_channel.h"
#include "tcpip_channel.h"

namespace ssh
{
    /* Function:        tcpip_channel
     * Description:     Implements the TCP/IP forwarding functionality
     */
    class tcpip_channel : public ssh_channel
    {
    public:
        tcpip_channel(ssh_connection * ssh, uint32 id, uint32 state) : ssh_channel(ssh,id, state) {
        }

        bool initalize(uint32 sender, uint32 window, uint32 maxsize, ConfigReader &) {return false;}
        virtual void OnFatalError() {}
        virtual void OnChannelSuccess() {}                                  
        virtual void OnChannelFailure(uint32 code, const char * reason) {}
        virtual void OnEof() {}                                         
        virtual void OnClosure() {} 
    
        bool isDataAvailable() {return false;}
        int UpdateWindow() {return 0;}
        size_t getInitialWindow() {return m_bufferSize;}
    protected:
        
        // read/write buffers.
        struct {
            byte * buff;
            size_t len, written, usage;
        } readbuff, writebuff;

        SocketLayer m_sock;
        uint32 m_id, m_senderId;
        size_t m_bufferSize;
    };

    // used for local tcp-ip forwarding.
    class local_tcp_forwarding : public tcpip_channel
    {
    public:
        // constructor, used for local forwarding.
        local_tcp_forwarding(SocketLayer &, ssh_connection *, uint32);
        int parse(const byte * dst, uint32);
        int write(byte *, uint32, uint32 &);

        void OnChannelSuccess();
        void OnChannelFailure(uint32 code, const char * reason);
    };

    // used for remote forwarding.
    class remote_tcp_forwarding : public tcpip_channel
    {
    public:
        // constructor, used for remote forwarding.
        remote_tcp_forwarding(uint32 channelId, const std::string & addr, uint32 port, ssh_connection *);
        int parse(const byte * dst, uint32);
        int write(byte *, uint32, uint32 &);
        int update();
        bool isDataAvailable();
        int UpdateWindow();
    protected:
        bool bConnected;
        int m_port;
        std::string m_addr;
    };

};

#endif