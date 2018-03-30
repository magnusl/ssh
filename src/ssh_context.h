#ifndef _SSH_CONTEXT_H_
#define _SSH_CONTEXT_H_

#include <string>
#include "ssh_hdr.h"

namespace ssh
{
    /* Function:        ssh_context
     * Description:     Contains context specific information.
     */
    class ssh_context
    {
    public:
        ssh_context() {
        }

        ssh_context(const char * protocol_version) : protocolVersion(protocol_version) {
        }

        bool load(const char *);        // loads the data from a file.

        std::string ciphers_client_to_server, ciphers_server_to_client,         // the ciphers
            hmacs_client_to_server,hmacs_server_to_client,                      // the keyed macs
            compression_client_to_server, compression_server_to_client,         // the compression algorithms
            kexs,                                                                   // the keyexchange algorithms
            hostkeys,                                                               // the hostkeys
            languages_client_to_server, languages_server_to_client;             // the languages

        std::string protocolVersion;        // the protocol version
        unsigned char kex_follows;
        uint32 reserved;
        unsigned char cookie[16];
        ssh_hdr kex_hdr;

    };
};

#endif