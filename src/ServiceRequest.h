#ifndef _SERVICEREQUEST_H_
#define _SERVICEREQUEST_H_

#include "RequestBase.h"
#include <string>
#include <iostream>

namespace ssh
{
    /* Class:           ServiceRequest
     * Description:     Used to request a service
     */
    class ServiceRequest : public RequestBase
    {
    public:
        // constructor
        ServiceRequest(const char * name) : m_service(name) 
        {
        }

        // Writes the service request to the stream
        bool write(IStreamIO * stream) 
        {
            if(!stream->writeByte(SSH_MSG_SERVICE_REQUEST) ||
                !stream->writeString(m_service))
            {
                std::cerr << "Could not write the service request to the stream" << std::endl;
                return false;
            }
            return true;
        }
    protected:
        std::string m_service;

    };
};

#endif