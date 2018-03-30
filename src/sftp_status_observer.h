#ifndef _SFTP_STATUS_OBSERVER_H_
#define _SFTP_STATUS_OBSERVER_H_

namespace sftp
{
    /*
     *
     */
    class sftp_status_observer
    {
    public:
        // Called when the client is connected
        virtual void OnConnect()                = 0;
        // Called when Host verification is required
        virtual void OnHostVerification()       = 0;
        // Called when the client is disconnected 
        virtual void OnDisconnect()             = 0;
        // Called when user authentication is required.
        virtual void OnAuthRequired()           = 0;
        // called when the client is authenticated.
        virtual void OnAuthSuccess()            = 0;
        // called if the authentication fails.
        virtual void OnAuthFailure()            = 0;
    };
};

#endif