#ifndef _TCPFORWARDINGOBJECT_H_
#define _TCPFORWARDINGOBJECT_H_

#include "ChannelObject.h"

namespace ssh
{
    /* Class:           TCPForwardingObject
     * Description:     Used for TCP/IP forwarding.
     */
    class TCPForwardingObject : public ChannelObject
    {
    public:
        TCPForwardingObject(const ForwardingInfo &, const std::string &, uint32);
        ~TCPForwardingObject();
        int write(unsigned char *, uint32, uint32 &);
        int parse(unsigned char * src, uint32 count);
        ObjectStatus status();
        void shutdown();                                
        void onClosed();                            
        void onEOF();
    };
};

#endif