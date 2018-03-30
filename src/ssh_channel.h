#ifndef _SSH_CHANNEL_H_
#define _SSH_CHANNEL_H_

#include <queue>

#include "definitions.h"
#include "types.h"
#include "IStreamIO.h"
#include "BoundedBuffer.h"
#include "requests.h"
#include "types.h"
#include "ConfigReader.h"

#define SSH_ACTIVE_CHANNEL      (1 << 0)
#define SSH_CLOSED_REMOTELY     (1 << 1)
#define SSH_CLOSED_LOCALLY      (1 << 2)
#define SSH_REMOTE_EOF          (1 << 3)
#define SSH_LOCAL_EOF           (1 << 4)
#define SSH_CHANNEL_DYING       (1 << 5)
#define SSH_CREATED_BY_SERVER   (1 << 6)
#define SSH_DELETE_INSTANCE     (1 << 7)

namespace ssh
{
    class ssh_connection;
    class ssh_transport;

    /* Class:           ssh_channel
     * Description:     A ssh channel, used to transfer data between the client and the server.
     */
    class ssh_channel
    {
    public:
        ssh_channel(ssh_connection *, uint32, uint32 );
        ~ssh_channel();
    
        // initalizes the channel.
        virtual bool initalize(uint32 sender, uint32 window, uint32 maxsize, ConfigReader &);
        // updates the connection.
        virtual int update() {return STATUS_SUCCESS;}

        /*
         * Notification functions.
         */
        virtual void OnRequestSuccess() {}
        virtual void OnRequestFailure() {}                                  
        virtual void OnFatalError()                                         = 0;
        virtual void OnChannelSuccess()                                 {m_state |= SSH_ACTIVE_CHANNEL; OnStateChange();}
        virtual void OnChannelFailure(uint32 code, const char * reason) {m_state |= SSH_CLOSED_REMOTELY; OnStateChange();}
        virtual void OnEof()                                            {m_state |= SSH_REMOTE_EOF; OnStateChange();}
        virtual void OnClosure()                                        {m_state |= SSH_CLOSED_REMOTELY; OnStateChange();}
        virtual void OnLocalClose()                                     {m_state |= SSH_CLOSED_LOCALLY; OnStateChange();}
        
        /*
         * Request functions. 
         */
        bool request_tty(const tty_settings *);                     // Requests a Pseduo Terminal
        bool request_X11(const x11_settings *);                     // requests X11 forwarding
        bool setenv(const char * name, const char * value);         // sets a environment variable
        bool request_shell();                                       // request a shell
        bool exec(const wchar_t * command);                         // execute a command
        bool exec_subsystem(const char * name);                     // executes a subsystem.

        /*
         * I/O for channel data.
         */
        virtual int parse(const byte * dst, uint32) = 0;
        virtual int write(byte *, uint32, uint32 &) = 0;
        virtual int parse_extended(const byte *, uint32, uint32) {return STATUS_SUCCESS;}

        // closes the channel.
        void close();
        /*
         * Misc.
         */
        virtual bool isDataAvailable() = 0; 
        virtual int UpdateWindow() = 0;                         

        uint32 getRemoteId() {return remote_id;}
        uint32 getLocalId() {return local_id;}
        uint32 getRemoteWindow() {return remote_window;}
        
        virtual size_t getInitialWindow() {return 0;}
        virtual size_t getPacketLimit() {return 32000;}     // return the default packet limit.

        // Must make the ssh connection a friend.
        friend class ssh_connection;
        friend class ssh_transport;

        // returns the channel state
        uint32 getState() {return m_state;}
    protected:

        void increase_localwindow(uint32);
        void increase_remotewindow(uint32);
        void decrease_localwindow(uint32);
        void decrease_remotewindow(uint32);

        void OnStateChange();
        std::string StateToString(uint32);

        ssh_connection * m_ssh;

        uint32 local_id,        // the local channel id 
            remote_id,          // the remote channel id
            local_window,       // the local window size
            remote_window,      // the remote window size
            remote_max,         // the remote packet limit.
            m_state;
    };
};
#endif