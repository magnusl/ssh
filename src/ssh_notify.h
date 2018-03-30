#ifndef _SSH_NOTIFY_H_
#define _SSH_NOTIFY_H_

#include "ssh_channel.h"
#include "HostIdent.h"
#include "ChannelObject.h"

namespace ssh
{
    /* Class:           ssh_notify
     * Description:     Used by the SSH subsystem to notify about events.
     */
    class ssh_notify
    {
    public:
        virtual void OnConnect(const char *)    {}
        virtual void OnConnectSuccess()         {}
        virtual void OnConnectFailed(const char *)                                  {}
        virtual void OnDisconnect(const wchar_t *)                                      {}
        virtual void OnKeyExchangeSuccess(const char *, const char *)   {}
        virtual void OnKeyExchangeFailure()     {}
        virtual void OnWarning(const wchar_t *) {}
        virtual void OnAuthBanner(const std::wstring &) {}

        virtual int OnUnknownHost(const std::string & host, const std::string & fp)             
        {
            std::stringstream ss;
            ss << "Do you accept the fingerprint: " << fp << " for the host: " << host << "?";
            int nret = MessageBoxA(0, ss.str().c_str(), "Accept host?", MB_YESNO);
            if(nret == IDYES)
                return HOST_ACCEPT_PERM;
            else if(nret == IDNO)
                return HOST_ERROR;
            return HOST_ERROR;
        }
        virtual int OnChangedFingerprint(const std::string &, const std::string &)      {return -1;}
        virtual void OnAuthRequired()                                                   {}
        virtual void OnAuthSuccess()                                                    {}
        virtual void OnAuthFailure()                                                    {}
        virtual void OnChannelCreation(ssh_channel *)                                   {}
        virtual void OnChannelCreationFailure(ssh_channel *)                            {}
        virtual void OnChannelRequestSuccess(ssh_channel *)                             {}
        virtual void OnChannelRequestFailure(ssh_channel *)                             {}
        virtual void OnChannelClose(ssh_channel *)                                      {}
        virtual void OnServiceAccept(const std::string &)                               {}
        virtual void OnRequestSuccess()                                                 {}
        virtual void OnRequestFailure()                                                 {}
    };
};

#endif