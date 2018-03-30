/* File:            handlers.cpp
 * Description:     Handles the different messages.
 * Author:          Magnus Leksell
 *
 * Copyright 2006-2007 Magnus Leksell, all rights reserved
 *****************************************************************************/

#include "ssh_connection.h"
#include "sshmessages.h"
#include <iostream>

using namespace std;

namespace ssh
{

    /* Function:        ssh_connection::handle_auth_msg
     * Description:     Handles a authentication message sent by the server.
     */
    int ssh_connection::handle_auth_msg(unsigned char type)
    {
        switch(type)
        {
        case  SSH_MSG_USERAUTH_FAILURE: 
            /* the authentication request failed */
            if(m_notify) m_notify->OnAuthFailure();
            return STATUS_SUCCESS;
        case SSH_MSG_USERAUTH_SUCCESS: 
            /* the authentication request succeded */
            if(m_notify) m_notify->OnAuthSuccess();
            return STATUS_SUCCESS;
        case SSH_MSG_USERAUTH_BANNER:
            {
                /* Got a authentication banner */
                wstring banner;
                if(!transportLayer.readWideString(banner))
                    return STATUS_FAILURE;
                if(m_notify) m_notify->OnAuthBanner(banner);
            }
            return STATUS_SUCCESS;
        default:
            /* Unknown message */
            cerr << "ssh_connection::handle_auth_msg(): unknown message." << endl;
            return STATUS_FAILURE;
        }
    }

    /* Function:        ssh_connection::handle_connection_msg
     * Description:     Handles a connection related message.
     */
    int ssh_connection::handle_connection_msg(unsigned char type)
    {
        switch(type)
        {
        case SSH_MSG_GLOBAL_REQUEST:
            /* The server sent a global request */
            /* TODO: write a reply to the server */
            return STATUS_FAILURE;
        case SSH_MSG_REQUEST_SUCCESS:
            {
                if(!m_globals.empty()) {
                    GlobalRequest * request = m_globals.top();
                    m_globals.pop();
                    request->success();
                    if(m_notify) m_notify->OnRequestSuccess();
                    delete request;
                }
                return STATUS_SUCCESS;
            }
        case SSH_MSG_REQUEST_FAILURE:
            {
                if(!m_globals.empty()) {
                    GlobalRequest * request = m_globals.top();
                    m_globals.pop();
                    request->failure();
                    if(m_notify) m_notify->OnRequestSuccess();
                    delete request;
                }
                return STATUS_SUCCESS;
            }
        default:
            cerr << "ssh_connection::handle_connection_msg(): Unknown message" << endl;
            return FATAL_ERROR;
        }
    }

    /* Function:        ssh_connection::handle_negotiation_msg
     * Description:     Handles a negotiation message.
     */
    int ssh_connection::handle_negotiation_msg(unsigned char type)
    {
        switch(type)
        {
        case SSH_MSG_KEXINIT:
            /* The server wants to perform a new keyexchange */
            return FATAL_ERROR; //transportLayer.perform_keyexchange();

        case SSH_MSG_NEWKEYS:
            cerr << "Protocol Error, got a SSH_MSG_NEWKEYS message, should only happen during the keyexchange process" << endl;
            return FATAL_ERROR;
        default:
            std::cerr << "Unknown Algorithm negotiation message received" << std::endl;
            return FATAL_ERROR;
        }
    }

    /* Function:        ssh_connection::handle_transport_msg
     * Description:     Handles a transport message.
     */
    int ssh_connection::handle_transport_msg(unsigned char type)
    {
        switch(type)
        {
        case SSH_MSG_SERVICE_ACCEPT:
            {
                /* The service request was accepted */
                std::string service_name;
                if(!transportLayer.readString(service_name)) { 
                    // Could not read the service name from the stream.
                    cerr << "Error while parsing the SSH_MSG_SERVICE_ACCEPT message" << endl;
                    return FATAL_ERROR;
                }
                // notify the client that the request was accepted.
                if(m_notify) m_notify->OnServiceAccept(service_name);
                return STATUS_SUCCESS;
            }
        case SSH_MSG_IGNORE:
            // ignore message
            return STATUS_SUCCESS;
        case SSH_MSG_UNIMPLEMENTED:
            // What should I do here?
            return FATAL_ERROR;
        case SSH_MSG_DEBUG:
            // Debug message, ignore it for now.
            return STATUS_SUCCESS;
        default:
            cerr << "handle_connection_msg(): Unknown connection messag: " << (uint32) type << endl;
            return FATAL_ERROR;
        }
    }


    /*
     *
     */
    int ssh_connection::handle_keyexchange_msg(unsigned char type)
    {
        return STATUS_SUCCESS;
    }
};