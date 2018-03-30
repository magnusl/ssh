#ifndef _PASSWORDREQUEST_H_
#define _PASSWORDREQUEST_H_

#include "PasswordRequest.h"

namespace ssh
{
    /* Class:           PasswordRequest
     * Description:     Used to perform password authentication.
     */   
    class PasswordRequest : public RequestBase
    {
    public:
        PasswordRequest(const wchar_t * user, const wchar_t * password)
            : m_user(user), m_password(password) {
        }

        // Writes the request to the stream
        bool write(IStreamIO * stream)
        {
            if(!stream->writeByte(SSH_MSG_USERAUTH_REQUEST) ||
                !stream->writeWideString(m_user) ||
                !stream->writeString("ssh-connection") ||
                !stream->writeString("password") ||
                !stream->writeByte(0) ||
                !stream->writeWideString(m_password))
            {
                std::cerr << "Could not write the password authentication request to the stream" << std::endl;
                return false;
            }
            return true;
        }
    protected:
        std::wstring m_password, m_user;
    };
};

#endif