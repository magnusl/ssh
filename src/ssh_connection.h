/* File:            ssh_connection.h
 * Description:     The class implements the SSH 2.0 Connection Layer.
 * Author:          Magnus Leksell
 *
 * Copyright 2006-2007 © Magnus Leksell, all rights reserved.
 *****************************************************************************/

#ifndef _SSH_CONNECTION_H_
#define _SSH_CONNECTION_H_

#include "ssh_transport.h"
#include "ConfigReader.h"
#include "definitions.h"
#include "requests.h"
#include <queue>
#include <map>
#include "ssh_channel.h"
#include <set>
#include "INotify.h"
#include "ssh_notify.h"
#include "RequestBase.h"
#include "ChannelRequest.h"
#include "ThreadLock.h"
#include "Event.h"
#include <map>
#include "ssh_channel.h"
#include "SynchronizedList.h"
#include "channel_action.h"
#include "GlobalRequest.h"
#include "ForwardingInfo.h"
#include <stack>
#include <list>

typedef std::map<uint32, ssh::ssh_channel *> ChannelList;
typedef std::map<uint32, ssh::ssh_channel *>::iterator ChannelIterator;

namespace ssh
{
    /* Class:           ssh_connection
     * Description:     Implements the SSH 2.0 Connection Protocol.
     *
     * Note:            The initalize function MUST be called before a call to
     *                  connect.
     */
    class ssh_connection
    {
    public:
        // constructor.
        ssh_connection(const char * addr, const char * port, ssh_notify *, Event * ev);
        ~ssh_connection();
        // loads the settings from a file.
        bool initalize(const char * file);
        // requests a specific service
        int request_service(const char * name);
        // performs a password authentication.
        int password_authentication(const wchar_t * name, const wchar_t * password);
        // allocates a channel, returns the id of the channel.
        ssh_channel * channel_alloc(ssh_channel *, const char * type);
        // used to request a specific type of channel
        bool request_channel(ssh_channel *, RequestBase *);
        // closes a channel
        int close_channel(uint32);
        // starts the SSH connection.
        int connect();
        // updates the connection, "drives" the SSH subsystem, could run in a separate thread.
        int run();
        // adds requets to the queue.
        int add_request(ssh::RequestBase *);
        // initiates the shutdown of the connection
        int shutdown();

        // registers a local forwarding.
        int register_local_forwarding(const char * localAddr, uint32 localPort, const char * host, uint32 hostPort);
        int unregister_local_forwarding(const char * localAddr, uint32 localPort);

        // registers a remote forwarding.
        int register_remote_forwarding(const char * bindaddr, uint32 bindport, const char * host, uint32 port);
        int unregister_remote_forwarding(const char * bindaddr, uint32 bindport);

        // returns the next channel id.
        uint32 next_channel_id();
    private:
        int execute_task();
        int perform_keyexchange(bool bServerInitiated);
        void cleanup();

        // handles a request.
        int handle_request();
        int handle_channel_request(ssh_request &);
        int channel_event();
        int check_channels();       
        bool change_channel_status(uint32 id, int state);
        bool find_channel(uint32 id, ssh_channel ** channel, int * state);
        void remove_channel(uint32 id, bool bDelete = false);
        int SendChannelData(ssh_channel * channel);
        int IncreaseLocalWindow(ssh_channel * channel, int bytes);

        int CheckIncomingSockets();

        /* Function that handles different types of messages */
        int handle_packet();                        
        int handle_auth_msg(unsigned char);         // Authentication messages
        int handle_channel_msg(unsigned char);      // channel messages
        int handle_connection_msg(unsigned char);   // Connection messages
        int handle_negotiation_msg(unsigned char);  // algorithm negotiation messages
        int handle_transport_msg(unsigned char);    // transport messages
        int handle_keyexchange_msg(unsigned char);  // keyexchange messages.
        int handle_portforwarding();

        ssh_transport   transportLayer;                         // the transport layer
        ConfigReader    config;                                 // the current configuration.
    
        std::map<uint32, ssh_channel *> m_channels;

        ssh_notify * m_notify;                              // the application supplied callback.
        SynchronizedList<RequestBase *> requests;
        std::list<ssh_channel *> m_queue;

        /* Locks */
        ThreadLock  m_channelLock;      // used to synchronize the channel lists

        /* Host information */
        std::string m_addr, m_port;             
        
        /* Stack with the outstanding requests */
        std::stack<GlobalRequest *> m_globals;

        /* Port forwarding */
        SynchronizedList<ForwardingInfo> localForwarding, remoteForwarding;

        /* Misc */
        ssh::Event * m_ev;
        uint32 state, channelId;
        time_t lastKeyexchange;         // the time the last keyexchange was performed.
        uint32 numGlobalRequests;
    };
};

#endif