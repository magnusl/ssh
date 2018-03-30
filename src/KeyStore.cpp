#include "KeyStore.h"
#include "AES.h"
#include "sha1.h"
#include <fstream>
#include <iostream>
#include "Base64.h"
#include <assert.h>

namespace ssh
{
    using namespace std;

    /* Function:        KeyStore::LoadKeys
     * Description:     Loads the encrypted public keys from a file.
     */
    int KeyStore::LoadKeys(const char * filename, const char * passphrase)
    {
        // load the database.
        std::ifstream file(filename, ios::binary);
        if(!file) {
            return -1;
        }
        file.seekg(0, ios::end);
        size_t size = file.tellg();
        if(size <= 20) {
            cerr << "KeyStore::LoadDatabase: Failed to load the publickey database, invalid size" << endl;
            return -1;
        }
        size_t length = size - 20;
        if(length % 16) {
            cerr << "KeyStore::LoadDatabase: Failed to load the database, the data is not a multiple of 16" << endl;
            return -1;
        }
        unsigned char digest[20];
        // now read the entire database into memory
        unsigned char * data = new unsigned char[length];
        file.seekg(0, ios::beg);
        file.read((char *) data, (std::streamsize) length); 
        file.read((char *) digest, 20);
        file.close();
        // now decrypt it
        if(!decrypt(data,length, passphrase)) {
            cerr << "KeyStore::LoadDatabase: Decryption failed" << endl;
            delete [] data;
            return -1;
        }
        // now verify it
        if(!verify(data, length, digest)) {
            cerr << "KeyStore::LoadDatabase: The verification failed, the supplied key was invalid" << endl;
            delete [] data;
            return BAD_PASSPHRASE;
        }

        ArrayStream stream(data, (uint32) length);
        bool ret = parse(stream);
        delete [] data;
        return (ret ? 0 : -1);
    }

    /* Function:        KeyStore::decrypt
     * Description:     Decrypts data using the supplied password. The password will be hashed with
     *                  SHA-1 and then use as a key to the AES-128 algorithm.
     */
    bool KeyStore::decrypt(unsigned char * data, size_t length, const char * password)
    {
        const char * iv = "deadcodedeadbabe";
        unsigned char key[20];
        // hash the passphrase to get the key.
        hash(password, key);
        AES aes(20); // 128 bit
        if(!aes.decryptInit(key, (const unsigned char *) iv)) {
            return false;
        }
        if(!aes.decrypt(data, data, (int) length))
            return false;
        return true;
    }

    /* Function:        KeyStore::hash
     * Description:     Hashes the supplied data with the SHA-1 algorithm
     */
    void KeyStore::hash(const char * passphrase, unsigned char * digest)
    {
        uint32 dlen = 20;
        sha1().hash((const unsigned char *)passphrase, (int) strlen(passphrase), digest,&dlen);
    }

    /* Function:        KeyStore::verify
     * Description:     Calculates the digest of the supplied data and then compares it with the supplied digest.
     */
    bool KeyStore::verify(const unsigned char * data, size_t length, unsigned char * digest)
    {
        unsigned char cdigest[20];
        uint32 clen = 20;
        // hash the data.
        sha1().hash(data,(int) length, cdigest, &clen);
        // compare the hashes
        if(memcmp(cdigest, digest, 20) == 0)
            return true;
        return false;
    }

    /* Function:        KeyStore::parse
     * Description:     Parses the loaded keyfile. The actual data will be padded to a multiple of 16, the reason
     *                  behind this is that the AES algorithm works on a 16 byte blocksize.
     */
    bool KeyStore::parse(ArrayStream & stream)
    {
        uint32 numkeys, blobsize;
        // first read the number of keys.
        if(!stream.readInt32(numkeys)) {
            cerr << "KeyStore::parse: Failed to read the number of keys" << endl;
            return false;
        }
        // read all the keys.
        for(uint32 i = 0; i < numkeys;i++)
        {
            KeyEntry ke;
            if(!stream.readString(ke.keytype) ||        // type, e.g. "ssh-dss"
                !stream.readString(ke.host) ||          // the host
                !stream.readWideString(ke.username) ||      // the username on the remote host
                !stream.readInt32(blobsize))            // the size of the keyblob.
            {
                cerr << "KeyStore::parse: Error while reading keydata" << endl;
                return false;
            }
            // now read the actual keyblob.
            ke.keyblob.resize(blobsize);
            if(!stream.readBytes(&ke.keyblob[0], blobsize))
            {
                cerr << "KeyStore::parse: Failed to read keyblob" << endl;
                return false;
            }
            // now add the entry to the list
            m_keys.push_back(ke);
        }
        return true;
    }

    /*
     *
     */
    int KeyStore::ImportKey_OpenSSH(const char * host, const char * filename, int mode)
    {
        std::ifstream file(filename);
        if(!file) {
        }
        string marker, line, hdr, key;
        if(!(line == "---- BEGIN SSH2 PUBLIC KEY ----")) {
        }
        if(!IsMarker(marker))
        {
        }
        // read the headers.
        getline(file, line);
        while(line.find_first_of(':') != string::npos)
        {
            hdr += line;
            while(line.at(line.length() - 1) == '\\') {
                getline(file, line);
                hdr += line;
            }
            // now parse the header.
                
            getline(file, line);
        }
        // this should be the first line of the key.
        while(line != "---- END SSH2 PUBLIC KEY ----") {
            key += line;
            getline(file, line);
        }
        // now decode the key. which is encoded in Base64.
        std::vector<unsigned char> keybuf(key.length());
        size_t clen = Base64_decode(key.c_str(), &keybuf[0]);
        assert(clen <= keybuf.size());
        keybuf.resize(clen);
        return 0;
    }   
};