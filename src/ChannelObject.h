#ifndef _CHANNELOBJECT_H_
#define _CHANNELOBJECT_H_

#include "types.h"
#include "IStreamIO.h"

namespace ssh
{
    class ssh_channel;

    /* Class:           ChannelObject
     * Description:     Each SSH channel has a ChannelObject attached to it. The
     *                  Channel object handles the channel communication.
     */
    class ChannelObject
    {
    public:

        enum ObjectStatus {StatusReady = 1, StatusIdle, StatusFinished, StatusError};
        // the virtual destructor.
        virtual ~ChannelObject() {}
        // initalizes the object.
        virtual bool init(ssh::ssh_channel *)                   = 0;
        // Writes data to a buffer
        virtual int write(unsigned char *, uint32, uint32 &)    = 0;
        // reads data from a buffer
        virtual int parse(unsigned char * src, uint32 count)    = 0;
        // returns the status of the channel object.
        virtual ObjectStatus status()                           = 0;
        virtual void shutdown()                                 = 0;

        /* 
         * Returns specific parameters to use for the current channel type.
         */
        virtual uint32 initialWindow()  {return 25000;}
        virtual uint32 packetSize()     {return 32000;}

        /*
         * Notification functions regarding channel operations
         */
        virtual void RequestSuccess() {}                            // called when a channel request was successful
        virtual void RequestFailure() {}                            // called when a channel request failed
        virtual void FatalError() {}                                // called when a fatal error occurrs.
        virtual void CreationFailure(uint32, const char *) {}       // called if the channel couldn't be created
        virtual void Created() {}                                   // called if the channel was created.
        virtual void OnClosure() = 0;                               // called when the channel is closed.
        virtual void OnEof() {}                                     // called when the server send a EOF message.

    };
};

#endif