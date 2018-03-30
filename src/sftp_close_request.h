#ifndef _SFTP_CLOSE_REQUEST_H_
#define _SFTP_CLOSE_REQUEST_H_

#include "sftp_request.h"
#include "sftp_definitions.h"
#include "sftp_handle.h"
#include "IStreamIO.h"
#include "definitions.h"
#include "sftp_errorcodes.h"
#include "sftp_messages.h"

namespace sftp
{
    /* Class:           sftp_close_request
     * Description:     Used to close files/directories.
     */
    class sftp_close_request : public sftp_request
    {
    public:
        sftp_close_request(sftp_raw_notify * notify, sftp_handle * handle,bool reply) : 
          sftp_request(notify), m_handle(handle), m_reply(reply) {
          }

          // processes data
        int parse_reply(unsigned char, ssh::IStreamIO *);
        // writes the request, returns the number of bytes written.
        int write_request(ssh::IStreamIO *, uint32);

    protected:
        sftp_handle * m_handle;
        bool m_reply;

    };
};

#endif