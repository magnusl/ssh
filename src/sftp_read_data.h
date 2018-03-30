#ifndef _SFTP_READ_DATA_H_
#define _SFTP_READ_DATA_H_


#include "sftp_request.h"
#include "sftp_definitions.h"
#include "sftp_handle.h"
#include "IStreamIO.h"
#include "definitions.h"
#include "sftp_errorcodes.h"
#include "sftp_messages.h"

namespace sftp
{
    /* Class:           sftp_read_data
     * Description:     Reads data from a remote file.
     */
    class sftp_read_data : public sftp_request
    {
    public:
        sftp_read_data(sftp_raw_notify * notify, unsigned char * buff, uint64 count, sftp_handle * handle) : sftp_request(notify), 
            m_buff(buff), m_size(count), m_bytesread(0), m_left(count), m_handle(handle) {
        }
        // processes data
        int parse_reply(unsigned char, ssh::IStreamIO *);
        // writes the request, returns the number of bytes written.
        int write_request(ssh::IStreamIO *, uint32);
    private:
        unsigned char * m_buff;
        uint64 m_size, m_bytesread, m_left;
        sftp_handle * m_handle;
    };
};

#endif