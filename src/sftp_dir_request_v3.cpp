#include "sftp_dir_request_v3.h"
#include "sftp_messages.h"
#include "sftp_definitions.h"
#include "sftp_errorcodes.h"
#include "definitions.h"
#include <assert.h>
#include <iostream>
#include "sftp.h"

using namespace std;

namespace sftp
{
    /* Function:        sftp_dir_request_v3::parse_reply
     * Description:     Parses the reply to the SSH_FXP_READDIR request. SFTP version 3 
     *                  implementation.
     */
    int sftp_dir_request_v3::parse_reply(unsigned char type, ssh::IStreamIO * stream)
    {
        if(type == SSH_FXP_NAME)
        {
            /* got one or more filenames  */
            uint32 count;
            if(!stream->readInt32(count)) {
                return FATAL_ERROR;
            }
            for(uint32 i = 0; i < count;++i)
            {
                sftp_file_version_3 * file = new (std::nothrow) sftp_file_version_3();
                if(!file) {
                    cerr << __FUNCTION__ << ": Memory allocation failed" << endl;
                    return FATAL_ERROR;
                }
                if(!file->read(stream)) {
                    cerr << __FUNCTION__ << ": Failed to read file information from the stream" << endl;
                    delete file;
                    return FATAL_ERROR;
                }
                // add the file to the list.
                m_dir->files.push_back(file);
            }
            return SFTP_REQUEST_PENDING;
        }
        else if(type == SSH_FXP_STATUS)
        {
            /* EOF or error */
            std::wstring msg;
            std::string language;
            uint32 code;
            if(!stream->readInt32(code) || !stream->readStringUTF8(msg) || !stream->readString(language)) {
                cerr << __FUNCTION__ << ": Failed to read the SSH_FXP_STATUS reply" << endl;
                return FATAL_ERROR;
            }
            switch(code)
            {
            case SSH_FX_EOF:
                /* no more files */
                // notify the application.
                if(m_notify) m_notify->OnDirectoryListing(m_dir);

                return SFTP_REQUEST_COMPLETE;
            case SSH_FX_PERMISSION_DENIED:
                /* permission denied */
            case SSH_FX_FAILURE :
            default:
                // notify the application.
                if(m_notify) m_notify->OnDirectoryListingFailure(code);
                return SFTP_REQUEST_COMPLETE;
            }
        }
        else {
            // got a unknown reply
            assert(false);
            return FATAL_ERROR;
        }
    }
};