#include "HostCache.h"
#include <fstream>
#include <iostream>
#include <string>

namespace ssh
{
    using namespace std;

    /* Function:        HostCache::load
     * Description:     Loads the host-cache from a file.
     */
    bool HostCache::load(const char * filename)
    {
        // open the file and load the entries.
        m_filename = filename;
        ifstream file(filename);
        if(!file) {
            // create the file instead
            ofstream hostfile(filename);
            if(!hostfile) return false;
            return true;
        }
        // now read the entries
        string line;
        while(getline(file, line))
        {
            if(line.empty()) continue;
            size_t delim = line.find_first_of(' ');
            if(delim == string::npos || (delim == line.length() - 1)) {
            }
            // now split the string into two parts, the host and the fingerprint
            string host = line.substr(0, delim);
            string fingerprint = line.substr(delim + 1);
            // should trim the strings.
            map<string,string>::iterator it = fingerprints.find(host);
            if(it != fingerprints.end()) return false;      // duplicate entries.
            // add the fingerprint for the host
            fingerprints[host] = fingerprint;
        }
        file.close();
        return true;
    }

    /* Function:        HostCache::getFingerprint
     * Description:     Retrives a fingerprint for a host.
     */
    bool HostCache::getFingerprint(const std::string & host, std::string & fp)
    {
        map<string,string>::iterator it = fingerprints.find(host);
        if(it == fingerprints.end()) {
            return false;
        }
        fp = it->second;
        return true;
    }

    /* Function:        HostCache::addFingerprint
     * Description:     Adds a fingerprint to the cache.
     */
    bool HostCache::addFingerprint(const std::string & host, const std::string & fp)
    {
        map<string,string>::iterator it = fingerprints.find(host);
        if(it != fingerprints.end()) 
            return false;
        fingerprints[host] = fp;
        return true;
    }

    /* Function:        HostCache::replaceFingerprint
     * Description:     Replaces a stored fingerprint.
     */
    bool HostCache::replaceFingerprint(const std::string & host, const std::string & fp)
    {
        map<string,string>::iterator it = fingerprints.find(host);
        if(it == fingerprints.end()) 
            return false;
        it->second = fp;
        return true;
    }

    /* Function:        HostCache::flush
     * Description:     Flushes the cache to disk.
     */
    bool HostCache::flush()
    {
        // open the file.
        ofstream file(m_filename.c_str());
        if(!file) 
            return false;
        // write all the fingerprints to the file
        for(map<string,string>::const_iterator it = fingerprints.begin(); it != fingerprints.end(); it++) {
            file << it->first << " " << it->second << endl;
        }
        file.close();
        return true;
    }
};