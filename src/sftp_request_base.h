#ifndef _SFTP_REQUEST_BASE_H_
#define _SFTP_REQUEST_BASE_H_

#include "IStreamIO.h"
#include "types.h"
#include "sftp_hdr.h"
#include <string>
#include "sftp_impl.h"

namespace sftp
{
    /* Class:           sftp_request_base
     * Description:     Baseclass for the different SFTP requests.
     */
    class sftp_request_base
    {
    public:
        sftp_request_base(sftp::sftp_impl * impl) : m_impl(impl) {
        }

        enum Status {StatusSuccess = 0, StatusFailure, StatusError, StatusIncomplete, StatusWaitingForMore};
        virtual ~sftp_request_base() {}                                     // virtual destructor
        virtual Status write(ssh::IStreamIO *, uint32) = 0;                     // writes the request to a stream
        virtual Status parse(const sftp_hdr & hdr, ssh::IStreamIO *) = 0;   // reads the reply from a stream
        const wchar_t * getErrorString() {return m_errorString.c_str();}
    
        // Don't let the derived classes modify the string directly
    protected:
        // They must use this function
        void SetErrorString(const wchar_t * error) {m_errorString = error;}
        sftp_impl * m_impl;
    private:
        // To access this string.
        std::wstring m_errorString;
    };
};

#endif