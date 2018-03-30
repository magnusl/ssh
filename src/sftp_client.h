#ifndef _SFTP_CLIENT_H_
#define _SFTP_CLIENT_H_

#include "sftp_file_transfer.h"
#include "ssh_connection.h"
#include "sftp_notify.h"
#include "ssh_notify.h"
#include "WorkThread.h"
#include "sftp_object.h"
#include "Event.h"

#define SFTP_WAITING_FOR_SUBSYSTEM          1

namespace sftp
{
    /* Class:           sftp_client
     * Description:     
     */
    class sftp_client : ssh::ssh_notify, sftp::sftp_notify
    {
    public:

        sftp_client();
        ~sftp_client();
        // creates a connection. Will notify the client with the OnConnect method.
        bool Connect(const char * addr, const char * port);
        void stop();

        bool dir(const wchar_t *, sftp_directory *);    // performs a directory listing.
        bool remove(const wchar_t * file);              // removes a single file.

        /* SSH notification functions */
        virtual void OnConnect(const char * addr);
        virtual void OnConnectFailed(const char * addr);
        virtual bool OnHostVerificationRequired(const ssh::HostIdent *);
        virtual void OnChannelRequestSuccess(ssh::ssh_channel *);
        virtual void OnChannelRequestFailure(ssh::ssh_channel *);
        virtual void OnChannelCreation(ssh::ssh_channel *);                         
        virtual void OnChannelCreationFailure(ssh::ssh_channel *);  
        virtual void OnAuthSuccess();
        //void OnAuthRequired();
        virtual void OnAuthFailure();
        //void OnServiceAccept(const std::string &);
        
        // closes the connection
        void close();

        //virtual void begin_authentication() = 0;  // the subclasses must implement this.

        // called when a fatal error occurrs
        virtual void fatal(const char *);

    protected:
        ssh::WorkThread     * m_worker;     // the workthread.
        ssh::ssh_connection * m_ssh;        // The SSH connection
        //ssh::ssh_channel  * m_sftp;       // the SFTP channel
        ssh::Event          m_stopEvent;
        sftp_object         * m_sftp;
        uint32              m_state;
        ssh::Event          m_startEvent;

        sftp_directory      m_dir;
    };
};

#endif