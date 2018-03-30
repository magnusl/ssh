/* File:            channels.cpp
 * Description:     Contains the functions in the connection layer that
 *                  deals with ssh channels.
 * Author:          Magnus Leksell
 *
 * Copyright © 2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/

#include "ssh_connection.h"
#include <map>
#include <iostream>
#include "ssh_channel.h"
#include "sshmessages.h"
#include "definitions.h"
#include <assert.h>
#include <cstdlib>
#include <memory>
#include "ChannelAllocRequest.h"
#include "SynchronizedContainer.h"
#include "TCPForwardingObject.h"
#include "ChannelOpenReply.h"
#include "ChannelClose.h"

void dump(char * name, unsigned char * data, uint32 len);

#define LOCK_REQUESTS()     m_requestLock.lock();
#define UNLOCK_REQUESTS()   m_requestLock.unlock();
#define LOCK_CHANNELS()     m_channelLock.lock();
#define UNLOCK_CHANNELS()   m_channelLock.unlock();
//#define ADD_REQUEST(req)  {LOCK_REQUESTS() requests.push(request); UNLOCK_REQUESTS()}
#define ADD_SSH_CHANNEL(c, id)\
{LOCK_CHANNELS(); m_channels[id] = c; UNLOCK_CHANNELS()}

namespace ssh
{

    using namespace std;

    /* Function:        ssh_connection::next_channel_id
     * Description:     Returns the next available channel id.
     */
    uint32 ssh_connection::next_channel_id()
    {
        LOCK_CHANNELS()
        uint32 id = channelId++;
        UNLOCK_CHANNELS()
        return id;
    }
    /* Function:        ssh_connection::channel_alloc
     * Description:     Requests a new channel.
     */
    ssh_channel * ssh_connection::channel_alloc(ssh_channel * channel, const char * type)
    {
        uint32 id = channel->getLocalId();
        // create the request.
        ChannelAllocRequest * req = new (std::nothrow) 
            ChannelAllocRequest(id,type,static_cast<uint32>(channel->getInitialWindow()), static_cast<uint32>(channel->getPacketLimit()));
        // add the new passive channel.
        ADD_SSH_CHANNEL(channel, id)
        requests.insert(req);
        return channel;
    }

    /* Function:        ssh_connection::request_channel
     * Description:     Used to request a specific type of channel.
     */
    bool ssh_connection::request_channel(ssh_channel * channel, RequestBase * request)
    {
        ADD_SSH_CHANNEL(channel, channel->getLocalId())
        requests.insert(request);
        return true;
    }

    /* Function:        ssh_connection::close_channel   
     * Description:     Closes a channel.
     */
    int ssh_connection::close_channel(uint32 id)
    {
        ssh_channel * channel;
        int state;
        if(!find_channel(id, &channel, &state)) return STATUS_FAILURE;
        // now change the state
        channel->OnLocalClose();
        if(!(state & SSH_CLOSED_REMOTELY)) {
            ChannelClose * request = new ChannelClose(id);
            add_request(request);
        } else {
            remove_channel((state & SSH_DELETE_INSTANCE) ? true : false);
        }
        // return success.
        return STATUS_SUCCESS;
    }

    /* Function:        ssh_connection::handle_channel_msg
     * Description:     Handles a channel related message
     */
    int ssh_connection::handle_channel_msg(unsigned char type)
    {
        switch(type)
        {
        case SSH_MSG_CHANNEL_OPEN:
            {
                string type;
                if(!transportLayer.readString(type)) return FATAL_ERROR;

                // only allow TCP/IP forwarding requests
                if(!(type == "forwarded-tcpip")) {
                    cerr << "ssh_connection::handle_channel_msg: ignoring channel request for unexpected channel type \"" << type << "\"" << endl;
                    return STATUS_SUCCESS;
                }
                if(!handle_portforwarding())
                    return STATUS_FAILURE;
                return STATUS_SUCCESS;
            }
        case SSH_MSG_CHANNEL_OPEN_CONFIRMATION:
            {
                /* the channel was created, read the channel settings from the stream */
                uint32 recipient, sender_channel, window_size, packet_limit;
                if(!transportLayer.readInt32(recipient) || 
                    !transportLayer.readInt32(sender_channel) || 
                    !transportLayer.readInt32(window_size) ||
                    !transportLayer.readInt32(packet_limit))
                {
                    cerr << "ssh_connection::handle_channel_msg: Could not read the channel open confirmation message" << endl;
                    return STATUS_FAILURE;
                }
                ssh_channel * channel;
                int state;
                if(!find_channel(recipient, &channel, &state)) 
                    return STATUS_FAILURE;
            
                if(state & SSH_ACTIVE_CHANNEL || state & SSH_CREATED_BY_SERVER) {
                    // server isn't allowed to send the open confirmation.
                    return STATUS_FAILURE;
                }
                // initalize the channel
                channel->initalize(sender_channel, window_size, packet_limit, config);
                // the channel was created successfully, this will change the state to active
                channel->OnChannelSuccess();
                // return success
                return STATUS_SUCCESS;
            }
        case SSH_MSG_CHANNEL_OPEN_FAILURE: /* failed to open the channel */
            {
                uint32 id, code;
                string reason, languages;
                ssh_channel * channel;
                if(!transportLayer.readInt32(id) ||             // channel identifier
                    !transportLayer.readInt32(code) ||          // error code
                    !transportLayer.readString(reason) ||       // error string
                    !transportLayer.readString(languages))      // language tag.
                {
                    return STATUS_FAILURE;
                }
                int state;
                if(!find_channel(id, &channel, &state)) return STATUS_FAILURE;
                if(state & SSH_ACTIVE_CHANNEL) {
                    // the channel is already opened.
                    return STATUS_FAILURE;
                }
                channel->OnChannelFailure(code, reason.c_str());
                // remove the channel
                remove_channel(id, (state & SSH_DELETE_INSTANCE) ? true : false);
                return STATUS_SUCCESS;
            }
        case SSH_MSG_CHANNEL_WINDOW_ADJUST:
            {
                /* adjust the windowsize */
                uint32 id, numbytes;
                if(!transportLayer.readInt32(id) || !transportLayer.readInt32(numbytes)) return FATAL_ERROR;
                ssh_channel * channel;
                int state;
                if(!find_channel(id, &channel, &state)) return STATUS_FAILURE;
                if(!(state & SSH_ACTIVE_CHANNEL)) return STATUS_FAILURE;
                // increase the window size.
                channel->increase_remotewindow(numbytes);
                // return success.
                return STATUS_SUCCESS;
            }
        case SSH_MSG_CHANNEL_DATA:      /* Channel Data */
            {
                uint32 id, numbytes;
                if(!transportLayer.readInt32(id) || !transportLayer.readInt32(numbytes))
                    return STATUS_FAILURE;
                ssh_channel * channel;
                int state;
                if(!find_channel(id, &channel, &state)) 
                    return STATUS_FAILURE;
                if(!(state & SSH_ACTIVE_CHANNEL) || (state & SSH_REMOTE_EOF) || (state & SSH_CLOSED_REMOTELY)) 
                    return STATUS_FAILURE;
                if(transportLayer.parse(channel,numbytes)) 
                    return STATUS_FAILURE;
                // decrease the window size
                channel->decrease_localwindow(numbytes);
                return STATUS_SUCCESS;
            }
        case SSH_MSG_CHANNEL_EXTENDED_DATA:
            {
                uint32 id, numbytes, type;
                if(!transportLayer.readInt32(id) || !transportLayer.readInt32(numbytes) || !transportLayer.readInt32(type))
                    return STATUS_FAILURE;
                ssh_channel * channel;
                int state;
                if(!find_channel(id, &channel, &state)) 
                    return STATUS_FAILURE;
                if(!(state & SSH_ACTIVE_CHANNEL) || (state & SSH_REMOTE_EOF) || (state & SSH_CLOSED_REMOTELY)) 
                    return STATUS_FAILURE;
                if(!(state & SSH_ACTIVE_CHANNEL)) {
                    DebugPrint("Received extended data for a non-active channel");
                    return STATUS_FAILURE;
                }
                // parse the data.
                if(transportLayer.parse_extended(channel,numbytes,type)) 
                    return STATUS_FAILURE;
                // extended data consumes window size
                channel->decrease_localwindow(numbytes);
                return STATUS_SUCCESS;
            }
        case SSH_MSG_CHANNEL_EOF:
            {
                // Channel EOF, no more channel data will be sent from the server
                uint32 id;
                if(!transportLayer.readInt32(id)) return FATAL_ERROR;
                ssh_channel * channel;
                int state;
                if(!find_channel(id, &channel, &state)) return STATUS_FAILURE;
                // update the state of the channel
                channel->OnEof();
                return STATUS_SUCCESS;
            }
        case SSH_MSG_CHANNEL_CLOSE: // The channel was closed by the server
            {
                uint32 id;
                if(!transportLayer.readInt32(id)) return STATUS_FAILURE;
                ssh_channel * channel;
                int state;
                if(!find_channel(id, &channel, &state)) return STATUS_FAILURE;
                if(state & SSH_CLOSED_LOCALLY) {    // we have already sent a close request.
                    // tell the channel that it was closed.
                    channel->OnClosure();
                    if(m_notify) m_notify->OnChannelClose(channel);
                    // remove the channel, and possible delete the instance.
                    remove_channel(id, (state & SSH_DELETE_INSTANCE) ? true : false);   
                } else {
                    // the remote host closed the channel, let the channel to any required operations.
                    channel->OnClosure();
                }
                return STATUS_SUCCESS;
            }
        case SSH_MSG_CHANNEL_SUCCESS: /* the request was successful */
            {
                uint32 id;
                if(!transportLayer.readInt32(id)) return STATUS_FAILURE;
                ssh_channel * channel;
                int state;
                if(!find_channel(id, &channel, &state)) return STATUS_FAILURE;
                // tell the channel that the request was successful.
                channel->OnRequestSuccess();
                return STATUS_SUCCESS;
            }
        case SSH_MSG_CHANNEL_FAILURE:   /* the request failed */
            {   
                uint32 id;
                if(!transportLayer.readInt32(id)) return STATUS_FAILURE;
                ssh_channel * channel;
                int state;
                if(!find_channel(id, &channel, &state)) return STATUS_FAILURE;
                channel->OnRequestFailure();
                return STATUS_SUCCESS;
            }
        default:
            /* ignore other messages */
            return STATUS_SUCCESS;
        }
    }

    /* Function:        ssh_connection::add_request
     * Description:     Adds a request to the request queue.
     */
    int ssh_connection::add_request(ssh::RequestBase * request)
    {
        // add the request.
        requests.insert(request);
        return STATUS_SUCCESS;
    }

    /* Function:        ssh_connection::remove_channel
     * Description:     Removes the channel.
     */
    void ssh_connection::remove_channel(uint32 id, bool bDelete)
    {
        LOCK_CHANNELS()
        map<uint32,ssh_channel * >::iterator it = m_channels.find(id);
        ssh_channel * channel = 0;
        if(it != m_channels.end()) {
            channel = it->second;
            m_channels.erase(it);
            m_queue.remove(channel);
        }
        UNLOCK_CHANNELS()
    }

    // finds a channel
    bool ssh_connection::find_channel(uint32 id, ssh_channel ** channel, int * state)
    {
        bool bRet;
        LOCK_CHANNELS()
        map<uint32,ssh_channel *>::iterator it = m_channels.find(id);
        if(it == m_channels.end()) {
            bRet = false;
        } else {
            bRet = true;
            *channel = it->second;
            *state = (*channel)->getState();
        }
        UNLOCK_CHANNELS();
        return bRet;
    }
};