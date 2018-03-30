/* File:            ssh_connection
 * Description:     Contains the implementation of the SSH2 Connection 
 *                  Protocol Layer.
 * Author:          Magnus Leksell
 * 
 * Copyright © 2006-2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/
#include <iostream>
#include "sshmessages.h"
#include "ssh_connection.h"
#include <assert.h>
#include "definitions.h"
#include "ServiceRequest.h"
#include "PasswordRequest.h"
#include "common.h"

namespace ssh
{
    using namespace std;

    /* Function:        ssh_connection::ssh_connection()
     * Description:     The constructor.
     */
    ssh_connection::ssh_connection(const char * addr, const char * port, ssh_notify * notify, Event * ev) 
        : m_ev(ev),
        transportLayer(notify)
    {
        //current_service_request = NULL;
        m_notify                = notify;

        m_addr      = addr;     // the address
        m_port      = port;     // the port number
        channelId   = 0;
    }

    /* Function:        ssh_connection::~ssh_connection
     * Description:     The destructor.
     */
    ssh_connection::~ssh_connection()
    {
        cerr << "ssh_connection::~ssh_connection" << endl;
        // delete all the requests.
        RequestBase * request;
        while(requests.pop_first(request)) {
            delete request;
        }
    }

    /* Function:        ssh_connection
     * Description:     Connects to the server and performs the initial keyexchange.
     */
    int ssh_connection::connect()
    {
        cerr << "Connecting" << endl;
        int nret = transportLayer.connect(m_addr.c_str(), m_port.c_str(), m_ev);
        if(nret != STATUS_SUCCESS) {
            cerr << "Failed to connect" << endl;
            if(m_notify) m_notify->OnConnectFailed(m_addr.c_str());
            return STATUS_FAILURE;
        }
        if(m_notify) m_notify->OnConnect(m_addr.c_str());

        /* now perform the keyexchange */
        cerr << "performing keyexchange" << endl;
        nret = transportLayer.perform_keyexchange(false);
        
        if(nret != STATUS_SUCCESS)      // the keyexchange failed.
        {
            if(nret == INVALID_HOSTKEY) {
                transportLayer.disconnect(L"Verification of hostkey failed",SSH_DISCONNECT_HOST_KEY_NOT_VERIFIABLE);
                SetLastError(SSH_DISCONNECT_HOST_KEY_NOT_VERIFIABLE);
                if(m_notify) m_notify->OnConnectFailed("Could not verify the hostkey provided by the server");
            } else {
                transportLayer.disconnect(L"The keyexchange failed", SSH_DISCONNECT_KEY_EXCHANGE_FAILED);
                SetLastError(SSH_DISCONNECT_KEY_EXCHANGE_FAILED);
                if(m_notify) m_notify->OnConnectFailed("The keyexchange failed");
            }
            return STATUS_FAILURE;
        }
        // connected and the host has been verified.
        if(m_notify != NULL) {
            m_notify->OnConnectSuccess();   
        }
        // Let the client know that we have connected,
        return STATUS_SUCCESS;
    }
    
    /* Function:        ssh_connection::perform_keyexchange
     * Description:     Initiates the keyexchange when 
     */
    int ssh_connection::perform_keyexchange(bool bServerInitiated)
    {
        int nret;
        if(transportLayer.sending_packet()) {
            // sending a packet, so continue.
            nret = transportLayer.sendpacket();
            if(nret < 0) return nret;
        }
        // send my keyexchange packet
        if(!transportLayer.exchange_kex(true))  // only send the client keyexchange packet
            return FATAL_ERROR; 
        
        if(!bServerInitiated) {
            while(true){
                // if the server didn't start the mess,wait until we get it's keyexchange packet
                nret = transportLayer.readpacket_nonblock();
                if(nret < 0) {
                    return nret;
                } else if(nret == PACKET_COMPLETE) {
                    // Read a packet, now check the type.
                    if(transportLayer.getMessageType() != SSH_MSG_KEXINIT) {
                        // handle the message
                        nret = handle_packet();
                        if(nret != STATUS_SUCCESS) {
                            if(m_notify) m_notify->OnDisconnect(L"");
                        }
                    } else {
                        // Got the keyexchange packet,so continue
                        break;
                    }
                }
            }
        } 
        // Let the transport layer continue.
        return transportLayer.perform_keyexchange(true);
    }

    /* Function:        ssh_connection::shutdown()
     * Description:     Stops the connection.
     */
    int ssh_connection::shutdown()
    {
        /* Close all the open channels */
        if(this->transportLayer.sending_packet()) {
            // Currently sending a packet, continue
            if(transportLayer.sendpacket() < 0) {
                // just disconnect.
                transportLayer.force_disconnect();
                return STATUS_FAILURE;
            }
        } else {
            // Not sending a packet, are we reading one.
            if(transportLayer.reading_packet()) {
                // reading a packet, complete it first
                if(transportLayer.readpacket() < 0) {
                    DebugPrint("Failed to read the rest of the packet");
                    transportLayer.force_disconnect();
                    return STATUS_FAILURE;
                }
                // handle the packet
                if(handle_packet() != STATUS_SUCCESS) {
                    transportLayer.force_disconnect();
                }
            } else {
                // not reading a packet, disconnect
                transportLayer.disconnect(L"Disconnected by the client", SSH_DISCONNECT_BY_APPLICATION);
            }
        }
        // notify the client.
        if(m_notify) m_notify->OnDisconnect(L"Connection closed");
        return STATUS_SUCCESS;
    }


    /* Function:        ssh_connection::load_settings
     * Description:     Loads the settings from a file.
     */
    bool ssh_connection::initalize(const char *file)
    {
        ConfigReader config;
        if(file != NULL) {
            if(!config.readFile(file)) return false;
        }
        // initalize the transport layer.
        if(!transportLayer.initalize(config))
            return false;
        return true;
    }

    /*
     *
     */
    int ssh_connection::run()
    {
        int nret = execute_task();
        // disconnect
        transportLayer.force_disconnect();
        cerr << "ssh_connection::run: shutting down" << endl;
        cleanup();
        if(m_notify) m_notify->OnDisconnect(L"Disconnected from server");
        return nret;
    }

    /* Function:        ssh_connection::cleanu
     * Description:     Performs the required cleanup.
     */
    void ssh_connection::cleanup()
    {
        m_channelLock.lock();
        for(std::map<uint32, ssh_channel * >::iterator it = m_channels.begin(); 
            it != m_channels.end();
            it++)
        {
            // tell the channel that it was closed
            it->second->OnClosure();
        }
        m_channelLock.unlock();
    }

    /* Function:        ssh_connection::update()
     * Description:     Updates the SSH connection. Returns STATUS_FAILURE if the connection
     *                  should be terminated. This function "drives" the SSH subsystem, should be
     *                  very often. A possible implementation could be to put it i a separate thread.
     */
    int ssh_connection::execute_task()
    {
        while(!m_ev->isSignaled())
        {
            int nret;
            /* This code sends a packet. It will continue send a packet, or check if a new packet should
             be sent.
             Modified 07/07/11, moved the different code snippets to separate functions, makes the 
                              code easier to mantain.
            */
            if(transportLayer.sending_packet()) {
                // already sending a packet, so continue.
                if(transportLayer.sendpacket_nonblock() < 0) {
                    /* A error occurred, notify the client */
                    DebugPrint(L"Failed to send packet");
                    return FATAL_ERROR;
                }
            }
            else {
                // not sending a packet.
                if(!requests.empty())  {
                    /* handle a request */
                    nret = handle_request();
                } else {
                    /* Handle the channels */
                    nret = check_channels();
                }
                if(nret != STATUS_SUCCESS) {
                    return nret;
                }
            }
            /* Now do the reading */
            nret = transportLayer.readpacket_nonblock();
            if(nret < 0)  {
                DebugPrint(L"Failed to read packet.");
                return nret;
            }  else if(nret == PACKET_COMPLETE) {
                // read a packet, now handle it.
                if(transportLayer.getMessageType() == SSH_MSG_KEXINIT) {
                    // The server initiated a keyexchange
                    perform_keyexchange(true);
                } else {
                    nret = handle_packet();
                    if(nret == DISCONNECTED) {
                        DebugPrint(L"Disconnected by remote host.");
                        return DISCONNECTED;
                    } 
                    else if(nret != STATUS_SUCCESS) {
                        DebugPrint(L"Failed to handle packet.");
                        return nret;
                    }
                }
            }
            // check for any incoming connection (local TCP/IP forwarding).
            nret = CheckIncomingSockets();
            if(nret != STATUS_SUCCESS) return nret;
            // yield here maybe?
            Sleep(1);
        }
        return STATUS_SUCCESS;
    }

    /*
     *
     */
    int ssh_connection::IncreaseLocalWindow(ssh_channel * channel, int bytes)
    {
        // write the packet
        transportLayer.newPacket();
        if(!transportLayer.writeByte(SSH_MSG_CHANNEL_WINDOW_ADJUST) || 
            !transportLayer.writeInt32(channel->getRemoteId()) ||
            !transportLayer.writeInt32(bytes)) 
        {
            return FATAL_ERROR;
        }
        // send it.
        if(transportLayer.sendpacket_nonblock() < 0) return FATAL_ERROR;
        // increase the local variable.
        channel->increase_localwindow(bytes);
        return STATUS_SUCCESS;
    }

    /* Function:        ssh_connection::SendChannelData
     * Description:     Sends the channel data.
     */
    int ssh_connection::SendChannelData(ssh_channel * channel)
    {
        // write the channel data.
        int nret = transportLayer.writeChannelData(channel);
        if(nret != STATUS_SUCCESS) return nret;
        // send the channel data.
        nret = transportLayer.sendpacket_nonblock();
        if(nret < 0) return nret;
        return STATUS_SUCCESS;
    }

    /* Function:        ssh_connection::check_channels
     * Description:     Checks the channels.
     */
    int ssh_connection::check_channels()
    {
        int nret = STATUS_SUCCESS;
        m_channelLock.lock();
        for(map<uint32, ssh_channel * >::iterator it = m_channels.begin(); 
            it != m_channels.end();
            it++)
        {
            ssh_channel * channel = it->second;
            uint32 state = channel->getState();
            if(state & SSH_CHANNEL_DYING)   // the channel is dying, ignore it.
                continue;
            // all channels are updated
            channel->update();
            if(!(state & SSH_ACTIVE_CHANNEL))   // the channel is not active.
                continue;
            
            int bytes = channel->UpdateWindow();
            if((bytes > 0) && !(state & SSH_REMOTE_EOF)) {      // no need to increase the local window if the host won't send any more data.
                nret = IncreaseLocalWindow(channel, bytes);
            } 
            else if(!(state & SSH_LOCAL_EOF) && channel->isDataAvailable()) {       // don't send any data if we have sent a EOF message.
                nret = SendChannelData(channel);
            }
            // break if a error occurred.
            if(nret != STATUS_SUCCESS) 
                break;
        }
        m_channelLock.unlock();
        return nret;
    }

    /* Function:        ssh_connection::request_service
     * Description:     Used to request a specific service from the server. 
     */
    int ssh_connection::request_service(const char * name)
    {
        // request the service
        ServiceRequest * request = new (std::nothrow) ServiceRequest(name);
        if(request == NULL) {
            std::cerr << "Could not allocate a service request" << std::endl;
            return FATAL_ERROR;
        }
        requests.insert(request);
        return REQUEST_PENDING;
    }

    /* Function:        ssh_connection::password_authentication
     * Description:     Performs password authentication.
     */
    int ssh_connection::password_authentication(const wchar_t * name, const wchar_t * password)
    {
        PasswordRequest * request = new (std::nothrow) PasswordRequest(name, password);
        if(request == NULL) {
            std::cerr << "Failed to allocate the password request" << std::endl;
            return FATAL_ERROR;
        }
        // add the request.
        requests.insert(request);
        return REQUEST_PENDING;
    }

    /* Function:        ssh_connection::handle_request
     * Description:     Handles a request.
     */
    int ssh_connection::handle_request()
    {
        RequestBase * request;
        if(!requests.pop_first(request))
            return FATAL_ERROR;

        // Write the request to the transport layer
        transportLayer.newPacket();
        if(!request->write(&transportLayer)) {
            std::cerr << "Could not write the request to the transport layer" << std::endl;
            delete request;
            return FATAL_ERROR;
        }
        if(request->global()) { // add the request on top of the stack.
            m_globals.push((GlobalRequest *) request);
        }
        // start sending the request
        if(transportLayer.sendpacket_nonblock() < 0)
            return STATUS_FAILURE;
        return STATUS_SUCCESS;
    }

    /* Function:        ssh_connection::handle_packet
     * Description:     Handles a incoming packet.
     */
    int ssh_connection::handle_packet()
    {
        unsigned char type;
        /* Read the message type */
        if(!transportLayer.readByte(type)) return STATUS_FAILURE;
        if(type == SSH_MSG_DISCONNECT) {
            // Disconnected by the server
            uint32 reason;
            string msg, language;
            if(!transportLayer.readInt32(reason) ||
                !transportLayer.readString(msg) ||
                !transportLayer.readString(language))
            {
                return FATAL_ERROR;
            }
            return DISCONNECTED;
        }
        if(TRANSPORT_LAYER_GENERIC(type))           return handle_transport_msg(type);
        else if(CHANNEL_RELATED_MESSAGES(type))     return handle_channel_msg(type);
        else if(AUTHENTICATION_MESSAGE(type))       return handle_auth_msg(type);
        else if(CONNECTION_PROTOCOL_GENERIC(type))  return handle_connection_msg(type);
        else if(ALGORITHM_NEGOTIATION(type))        return handle_negotiation_msg(type);
        else if(KEYEXCHANGE_MESSAGE(type))          return handle_keyexchange_msg(type);
        else                                        return STATUS_FAILURE;
    }

    /* Function:        ssh_connection::CheckIncomingSockets
     * Description:     Checks for any incoming sockets.
     */
    int ssh_connection::CheckIncomingSockets()
    {
        if(!localForwarding.empty()) {
            LocalForwardingCheck check(this);
            // check all the sockets for any incoming connections.
            localForwarding.foreach(check);
        }
        return STATUS_SUCCESS;
    }
};