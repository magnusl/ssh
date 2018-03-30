/* File:            ssh_channel.cpp
 * Description:     Contains the functions relating to the SSH channel
 * Author:          Magnus Leksell
 *
 * Copyright © 2006-2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/

#include "ssh_channel.h"
#include "ssh_connection.h"
#include "types.h"
#include "ssh_transport.h"
#include "definitions.h"
#include "ExecRequest.h"
#include <iostream>
#include "SubsystemExec.h"

using namespace std;

namespace ssh
{
    const char * state_table[] = {
        "SSH_ACTIVE_CHANNEL",
        "SSH_CLOSED_REMOTELY",
        "SSH_CLOSED_LOCALLY",
        "SSH_REMOTE_EOF",
        "SSH_LOCAL_EOF"
    };

    /* Function:        ssh_channel::ssh_channel    
     * Description:     Initalizes the channel
     */
    ssh_channel::ssh_channel(ssh_connection * con, uint32 id, uint32 state) : m_ssh(con)                // the SSH connection 
    {
        local_id = id;
        local_window = 0;
        remote_window = 0;
        remote_max = 0;
        m_state = state;
    }

    /* Function:        ssh_channel::~ssh_channel
     * Description:     Destructor, performs the required cleanup.
     */
    ssh_channel::~ssh_channel()
    {
    }

    /* Function:        ssh_channel::initalize
     * Description:     Initalizes the channel
     */
    bool ssh_channel::initalize(uint32 sender, uint32 window, uint32 maxsize, ConfigReader & config)
    {
        remote_window   = window;
        remote_max      = maxsize;
        remote_id       = sender;
        return true;
    }

    /* Function:        ssh_channel::OnStateChange
     * Description:     Called when the state of the channel changes, used for debugging.
     */
    void ssh_channel::OnStateChange()
    {
#ifdef _DEBUG
        cerr << "Channel #" << local_id << " state change to " << StateToString(m_state) << endl;
#endif
    }

    /* Function         ssh_channel::StateToString
     * Description:     Converts the state to a string representation.
     */
    string ssh_channel::StateToString(uint32 state)
    {
        stringstream ss;
        ss << "{";
        if(state & SSH_ACTIVE_CHANNEL) {ss << "SSH_ACTIVE_CHANNEL";} else {ss << "SSH_PASSIVE_CHANNEL";}
        for(size_t i = 1;i < 5;i++)
            if(state & (1 << i)) ss << state_table[i];
        ss << "}";
        return ss.str();
    }

    /* Function:        ssh_channel::exec
     * Description:     Requests the remote execution of a command.
     */
    bool ssh_channel::exec(const wchar_t * cmdStr)
    {
        // Create the request.
        ExecRequest * request = new (std::nothrow) ExecRequest(cmdStr,local_id, remote_id);
        if(request == NULL) return false;
        // Now add the request to the connections request queue
        if(m_ssh->add_request(request) != STATUS_SUCCESS) return false;
        return true;
    }

    /* Function:        ssh_channel::request_tty
     * Description:     Used to request a TTY.
     */
    bool ssh_channel::request_tty(const tty_settings * tty)
    {
        return true;
    }

    /* Function:        ssh_channel::request_X11
     * Description:     Requests X11 forwarding.
     */
    bool ssh_channel::request_X11(const x11_settings *)
    {
        return false;
    }

    /* Function:        ssh_channel::request_shell
     * Description:     Requests a shell.
     */
    bool ssh_channel::request_shell()
    {
        return false;
    }

    /* Function:        ssh_channel::exec_subsystem
     * Description:     Executes a subsystem
     */
    bool ssh_channel::exec_subsystem(const char * name)
    {
        // Create the request.
        SubsystemExec * request = new (std::nothrow) SubsystemExec(local_id,remote_id, name);
        if(request == NULL) return false;
        // Now add the request to the queue
        if(m_ssh->add_request(request) != STATUS_SUCCESS) return false;
        return true;
    }   

    /* Function:        ssh_channel::increase_remotewindow
     * Description:     Increases the remote window size.
     */
    void ssh_channel::increase_remotewindow(uint32 num)
    {
        remote_window = static_cast<uint32>(min(static_cast<uint64>(remote_window) + static_cast<uint64>(num),
            0xFFFFFFFF));
    }

    /* Function:        ssh_channel::increase_localwindow
     * Description:     Increases the local window size.
     */
    void ssh_channel::increase_localwindow(uint32 num)
    {
        local_window = static_cast<uint32>(min(static_cast<uint64>(local_window) + static_cast<uint64>(num),
            0xFFFFFFFF));
    }

    /* Function:        ssh_channel::decrease_localwindow
     * Description:     Decreases the local window size, happens when channel data is read from the server.
     */
    void ssh_channel::decrease_localwindow(uint32 num)
    {
        assert(local_window >= num);
        local_window = (num > local_window ? 0 : (local_window - num));
    }

    /* Function:        ssh_channel::decrease_remotewindow
     * Description:     Decreases the remote window size, happens when data is written to the channel
     */
    void ssh_channel::decrease_remotewindow(uint32 num)
    {
        assert(remote_window >= num);
        remote_window = (num > remote_window ? 0 : (remote_window - num));
        //cerr << "decreasing the remote window with " << num << " to: " << remote_window << std::endl;
    }

    /* Function:        ssh_channel::close 
     * Description:     Closes the channel
     */
    void ssh_channel::close()
    {
        m_ssh->close_channel(local_id);
    }

};