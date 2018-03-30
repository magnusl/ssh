#ifndef _IFILE_H_
#define _IFILE_H_

#include "types.h"

namespace GenericTransfer
{
    /* Class:           IFile
     * Description:     Interface class for a file.
     */
    class IFile
    {
    public:
        virtual bool isDirectory()      = 0;
        virtual uint64 getSize()        = 0;
        const wchar_t * getFilename()   = 0;
        const wchar_t * getPath()       = 0;
        const wchar_t * getFullPath()   = 0;
    };
};

#endif