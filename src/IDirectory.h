#ifndef _IDIRECTORY_H_
#define _IDIRECTORY_H_

#include "types.h"
#include <list>
#include "IFile.h"

namespace GenericTransfer
{
    /* Function:        IDirectory
     * Description:     Interface class for a directory listing. Represents the contents
     *                  of a directory, not just the directory.
     */
    class IDirectory
    {
    public:
        // returns the number of files in the directory.
        uint64 getFileCount();
        // returns the name of the directory
        const wchar_t * getName();
        // returns the path to the directory without the directory name.
        const wchar_t * getPath();
        // returns the path including the directory name
        const wchar_t * getFullPath();
        // the list with the files.
        std::list<IFile *> m_files;
    };
};

#endif