/* File:            io_functions.cpp
 * Description:     Contains the implementation for the I/O functions for the
 *                  ssh transport protocol layer.
 * Author:          Magnus Leksell
 *
 * Copyright © 2006-2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/

#include "ssh_transport.h"
#include "definitions.h"
#include "ssh_channel.h"
#include <iostream>
#include "sshmessages.h"

using namespace std;

namespace ssh
{
    /* Function:        ssh_transport::writeBytes
     * Description:     Writes data to the outgoing packet.
     */
    bool ssh_transport::writeBytes(const unsigned char * data, uint32 len)
    {
        if(len == 0) return false;
        if((sendState.msg.payload_usage + len) > MAX_PAYLOAD_SIZE)
            return false;
        memcpy(sendState.msg.payload + sendState.msg.payload_usage, data, len);
        sendState.msg.payload_usage += len;
        return true;
    }

    /* Function:        ssh_transport::readBytes
     * Description:     Reads data from the message.
     */
    bool ssh_transport::readBytes(unsigned char * data, uint32 len)
    {
        if(len == 0) return false;
        if((readState.position + len) > readState.msg.payload_usage)
            return false;
        memcpy(data, readState.msg.payload + readState.position, len);
        readState.position += len;
        return true;
    }

    /* Function:        ssh_transport::writeChannelData
     * Description:     Writes the channel data to the output buffer.
     */
    int ssh_transport::writeChannelData(ssh_channel * channel)
    {
        uint32 numbytes;
        newPacket();
        int nret;
        unsigned char * type = sendState.msg.payload;
        uint32 * id = (uint32 *)(type + sizeof(unsigned char));
        uint32 * data_size = (id + 1);                          // channel + sizeof(uint32)
        unsigned char * data = (unsigned char *) (data_size + 1);   // data_size + sizeof(uint32)

        // Set the values
        *type       = SSH_MSG_CHANNEL_DATA;
        *id = __htonl32(channel->getRemoteId());

        uint32 size = min(channel->getRemoteWindow(), MAX_PAYLOAD_SIZE);
        nret = channel->write(data, MAX_PAYLOAD_SIZE, numbytes);
        if(nret < 0) {
            DebugPrint(L"Failed to write the channel data")
            return FATAL_ERROR;
        } else if(nret == STATUS_ACTION_ABORTED) {
            return STATUS_ACTION_ABORTED;
        }
        // The channel data was written successfully, now decrease the remote window
        channel->decrease_remotewindow(numbytes);

        // Encode the data as a string.
        *data_size = __htonl32(numbytes);
        // now update the total payload size
        sendState.msg.payload_usage = /* "header */ 9 + /* channel data */ numbytes; 
        return STATUS_SUCCESS;
    }
};