#ifndef _SFTP_DIRECTORY_H_
#define _SFTP_DIRECTORY_H_

#include "file_attributes.h"
#include <string>
#include <vector>
#include <iostream>
namespace sftp
{
    typedef std::pair<std::wstring, file_attributes *> sftpFile;
    typedef std::vector<sftpFile> DirectoryFiles;

    /* Class:           sftp_directory
     * Description:     Contains the directory lisiting of a directory.
     */
    class sftp_directory
    {
    public:

        // Add a file to the directory
        void addFile(const std::wstring filename,       // The filename 
                    file_attributes * attribs)          // The attributes.
        {
            m_files.push_back(sftpFile(filename, attribs));
        }

        std::wstring path;                              
        DirectoryFiles m_files;
    };

    std::ostream & operator << (std::ostream &, const sftp_directory);
};

#endif