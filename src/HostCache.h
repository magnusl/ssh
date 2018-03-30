#ifndef _HOSTCACHE_H_
#define _HOSTCACHE_H_

#include <string>
#include <map>

#define HOST_ACCEPT_TEMP        0
#define HOST_ACCEPT_PERM        1
#define HOST_REPLACE            2
#define HOST_ERROR              -1


namespace ssh
{
    /* Class:           HostCache
     * Description: 
     */
    class HostCache
    {
    public:
        // loads the file.
        bool load(const char *);
        // retrives the fingerprint of the host.
        bool getFingerprint(const std::string &, std::string &);
        // Adds a fingerprint for the host. Will fail if the host already exists.
        bool addFingerprint(const std::string &, const std::string &);
        // replaces the old fingerprint with the new one.
        bool replaceFingerprint(const std::string &, const std::string &);
        // flushes the entries to the host file.
        bool flush();
    protected:
        std::map<std::string, std::string> fingerprints;
        std::string m_filename;
    };
};

#endif