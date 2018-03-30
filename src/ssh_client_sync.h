#ifndef _SSH_CLIENT_SYNC_H_
#define _SSH_CLIENT_SYNC_H_

#include "ssh_connection.h"
#include "WorkThread.h"

namespace ssh
{
    /* Class:           ssh_client_sync
     * Description: 
     */
    class ssh_client_sync : public ssh::ssh_notify
    {
    public:
        ssh_client_sync();
        ~ssh_client_sync();
        // connects to the server
        int connect(const char *, const char *,const char * config = NULL);
        // requests a service. The connection will be closed if the request fails.
        int request_service(const char *);
        // performs a password authentication attempt.
        int authenticate(const wchar_t * user, const wchar_t * password);
        // shuts down the connection, must be called!
        void shutdown();
        // returns a pointer to the ssh_connecion
        ssh_connection * source() {return m_ssh;}
        
        // reply functions.
        void OnConnectSuccess();
        void OnConnectFailed(const char *);
        void OnAuthSuccess();
        void OnAuthFailure();
        void OnServiceAccept(const std::string &);
        void OnDisconnect(const wchar_t *);
        void OnRequestSuccess();
        void OnRequestFailure();
    protected:
        
        // sets the last error
        void setLastError(int);
        // gets the last error.
        int getLastError();

        bool isActive();

        ssh_connection *    m_ssh;          // the ssh connection.
        WorkThread *        m_worker;       // the primary work-thread
        ssh::Event          m_killEv;       
        ThreadLock          m_lock;
        int                 m_code;
    };
};

#endif