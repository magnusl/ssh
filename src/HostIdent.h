#ifndef _HOSTIDENT_H_
#define _HOSTIDENT_H_

#include <string>

namespace ssh
{
    /* Class:           HostIdent
     * Description:     Used to identify a host.
     */
    class HostIdent
    {
    public:
        HostIdent(const char * host) : m_host(host) {
        }
        // Returns the host name. 
        const std::string & getHost() const {return m_host;}
        // Returns a stringified version of the host keys
        virtual const std::string & toString() const = 0;
    private:
        std::string m_host;
    };
};

#endif