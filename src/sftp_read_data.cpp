#include "sftp_read_data.h"
#include <iostream>
#include <cstdlib>
#include <assert.h>

#define MAX_READ_SIZE       33000
using namespace std;

namespace sftp
{
    /* Function:        sftp_read_data::parse_reply
     * Description:     Parses the reply from the read request.
     */
    int sftp_read_data::parse_reply(unsigned char type, ssh::IStreamIO * stream)
    {
        switch(type)
        {
        case SSH_FXP_DATA:
            {
                uint32 len;
                unsigned char eof_flag;
                if(!stream->readInt32(len)) {
                    return FATAL_ERROR;
                }
                if(len == 0) {
                    cerr << __FUNCTION__ <<": Got a empty data reply" << endl;
                    return SFTP_REQUEST_PENDING;
                }
                uint64 numbytes = min(m_left, (uint64) len);
                if(!stream->readBytes(m_buff + m_bytesread, numbytes)) {
                    cerr << __FUNCTION__ <<": Failed to read the data reply" << endl;
                    return FATAL_ERROR;
                }
                if(stream->readByte(eof_flag)) {
                    /* End of file marker */
                }
                m_bytesread += numbytes;
                assert(m_left >= numbytes);
                m_left -= numbytes;
                if(left == 0) {
                    // done reading.
                    if(m_notify) m_notify->OnReadSuccess(m_handle);
                    return SFTP_REQUEST_COMPLETE;
                }
                return SFTP_REQUEST_PENDING;
            }
        case SSH_FXP_STATUS:
            {
                uint32 code;
                std::wstring reason;
                std::string languages;
                if(!stream->readInt32(code) || !stream->readStringUTF8(reason) || !stream->readString(languages))
                {
                    cerr << __FUNCTION__ << ": Failed to read the reply" << endl;
                    return FATAL_ERROR;
                }
                switch(code)
                {
                case SSH_FX_EOF:
                case SSH_FX_FAILURE:
                    if(m_notify) m_notify->OnReadFailure(m_handle);
                    return FATAL_ERROR;
                }   
            }
        default:
            cerr << __FUNCTION__ <<": Unknown reply to read data request" << endl;
            return FATAL_ERROR;
        }
    }

    /*
     *
     */
    int sftp_read_data::write_request(ssh::IStreamIO * stream, uint32 id)
    {
        if(!stream->writeByte(SSH_FXP_READ) ||
            !stream->writeInt32(id) ||
            !m_handle->write(stream) ||
            !stream->writeInt64(m_bytesread) ||
            !stream->writeInt32((uint32) min(min(m_left, (uint64) MAX_READ_SIZE), (uint64)0xFFFFFFFF)))
        {
            cerr << "Failed to write the read request to the stream" << endl;
            return FATAL_ERROR;
        }
        return STATUS_SUCCESS;
    }
};