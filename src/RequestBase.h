#ifndef _REQUESTBASE_H_
#define _REQUESTBASE_H_

#include "IStreamIO.h"

namespace ssh
{
    /* Class:           RequestBase
     * Description:     Baseclass for the SSH requests.
     */
    class RequestBase
    {
    public:
        virtual ~RequestBase() {}
        // Writes the request to a stream 
        virtual bool write(IStreamIO *) = 0;
        virtual bool global() const {return false;}
    };
};


#endif