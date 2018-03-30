#ifndef _SUBSYSTEMEXEC_H_
#define _SUBSYSTEMEXEC_H_

#include "ChannelRequest.h"
#include "sshmessages.h"
#include <string>
#include <iostream>

namespace ssh
{
    /* Class:           SubsystemExec
     * Description:     Used to request the execution of a remote subsystem like "SFTP".
     */
    class SubsystemExec : public ChannelRequest
    {
    public:
        /* Function:        Subsystem::SubsystemExec
         * Description:     Constructor, performs the required initalization.
         */
        SubsystemExec(uint32 local, uint32 remote, const char * name) : 
          ChannelRequest(local, remote) , m_name(name)
        {
        }

        /* Funtion:         SubsystemExec::write
         * Description:     Writes the request to the stream. 
         */
        bool write(IStreamIO * stream)
        {
            if(!stream->writeByte(SSH_MSG_CHANNEL_REQUEST) ||       // message type
                !stream->writeInt32(m_remote) ||                    // the channel
                !stream->writeString("subsystem") ||                
                !stream->writeByte(1) ||                            // TRUE, want a reply
                !stream->writeString(m_name))                       // the subsystem name.
            {
                // Could not write the request to the stream
                std::cerr << "Could not write the Subsystem exec request to the stream" << std::endl;
                return false;
            }
            return true;
        }
    private:
        std::string m_name;
    };
};

#endif