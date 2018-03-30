#ifndef _GLOBALREQUEST_H_
#define _GLOBALREQUEST_H_

#include "RequestBase.h"

namespace ssh
{
    class ssh_connection;
    /* Class:           GlobalRequest
     * Description:     A global requests affects the state of the entire connection.
     */
    class GlobalRequest : public RequestBase
    {
    public:
        GlobalRequest(ssh_connection * ssh) : m_ssh(ssh){
        }
        // called if the global request is successful
        virtual void success() {}
        // called if the global request fails.
        virtual void failure() {}
        bool global() const {return true;}
    protected:
        ssh_connection * m_ssh;
    };
};

#endif