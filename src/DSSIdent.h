#ifndef _DSSIDENT_H_
#define _DSSIDENT_H_

#include "HostIdent.h"
#include "dss.h"
#include <string>

namespace ssh
{
    /* Class:           DSSIdent
     * Description:     Digital Signature Standard identity.
     */
    class DSSIdent : public HostIdent
    {
    public:
        DSSIdent(const ssh::key * key, const char * host)
        {
            std::stringstream ss(m_pubkey);
            ss << "p = " << key->dss.p << ", q = " << key->dss.q <<", g = " << key->dss.g << ", y = " << key->dss.y;
        }
        const std::string & toString() const;
    protected:
        std::string m_pubkey;
    };
};

#endif