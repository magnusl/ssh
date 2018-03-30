#ifndef _SFTP_FILE_VERSION_3_H_
#define _SFTP_FILE_VERSION_3_H_

#include "sftp_file_base.h"
#include "attributes_v3.h"
#include <string>
#include <list>
#include "types.h"

namespace sftp
{
    /* Class:           sftp_file_version_3
     * Description:     SFTP version 3 file type.
     */
    class sftp_file_version_3 : public sftp_file_base
    {
    public:
        bool read(ssh::IStreamIO *);
        bool write(ssh::IStreamIO *);

    protected:
        std::string     longname;
        attributes_v3   attributes;
    };
};

#endif