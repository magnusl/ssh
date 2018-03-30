#include "sftp_file_version_3.h"
#include "sftp_definitions.h"

namespace sftp
{
    /* Function:        sftp_file_version_3::read
     * Description:     Reads the file information from the stream. SFTP Version 3 ONLY.
     */
    bool sftp_file_version_3::read(ssh::IStreamIO * stream)
    {
        /* read the filename and longname */
        if(!stream->readStringUTF8(filename) || !stream->readString(longname) || !attributes.read(stream))
            return false;
        return true;
    }

    /*
     *
     */
    bool sftp_file_version_3::write(ssh::IStreamIO *)
    {
        return false;
    }
};