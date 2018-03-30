#ifndef _KEYSTORE_H_
#define _KEYSTORE_H_

#include <vector>
#include <string>
#include <list>
#include "ArrayStream.h"

#define BAD_PASSPHRASE      1

namespace ssh
{
    struct KeyEntry
    {
        std::string host, keytype;
        std::wstring username;
        std::vector<unsigned char> keyblob;
    };

    /* Class:           KeyDatabase
     * Description:     
     */
    class KeyStore
    {
    public:
        // loads the database.
        int LoadKeys(const char * filename, const char * passphrase);
        // Imports a key stored in OpenSSH format.
        int ImportKey_OpenSSH(const char * host, const char * filename, int mode);
        //const PublicKey * GetUserKey(const char * host, const wchar_t * user, const char * type = NULL);
    protected:
        bool verify(const unsigned char *, size_t, unsigned char *);
        bool decrypt(unsigned char *, size_t, const char *);
        bool parse(ArrayStream &);
        void hash(const char *, unsigned char *);
        bool IsMarker(const std::string &);

        std::list<KeyEntry> m_keys;     // the loaded keys.
    };
};

#endif