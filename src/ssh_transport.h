#ifndef _SSH_TRANSPORT_H_
#define _SSH_TRANSPORT_H_

#include "algorithms.h"
#include "ConnectionState.h"
#include "ssh_context.h"
#include "endpoint_info.h"
#include "algorithm_list.h"
#include "kex_base.h"
#include "hostkey_base.h"
#include "SocketLayer.h"
#include "definitions.h"
#include "types.h"
#include "IStreamIO.h"
#include "ConfigReader.h"
#include "Event.h"
#include <ctime>
#include "HostCache.h"
#include "ssh_notify.h"

namespace ssh
{
    class ssh_channel;

    /* Class:           ssh_transport
     * Description:     Implements the SSH 2.0 Transport Layer.
     */
    class ssh_transport : public IStreamIO
    {
    public:
        ssh_transport(ssh_notify *);
        ~ssh_transport();

        int connect(const char * addr, const char * port, Event * ev = NULL);

        /* Send/Read functions */
        int readpacket_nonblock();
        int sendpacket_nonblock();
        int sendpacket(int timeout = DEFAULT_SEND_TIMEOUT);
        int readpacket(int timeout = DEFAULT_READ_TIMEOUT);

        // initalizes the transport layer.
        bool initalize(ConfigReader &);

        /* exchanges packets */
        int exchangePackets();                          // exchanges packets
        int exchangeAndExpect(unsigned char);
        int exchangeProtocolVersions();             // exchanges the protocol messages
        int exchangeKex();  

        /* key exchange */
        bool exchange_kex(bool bOnlyClientKex);
        int perform_keyexchange(bool bPacketsExchanged);
        int parse_server_kex();
        bool decide_algorithms(algorithm_list & list);
        int exchange_protocol_versions();

        /* starts a new packet */
        void newPacket();

        /* I/O functions */
        bool writeBytes(const unsigned char *, uint32);
        bool readBytes(unsigned char *, uint32);

        //bool decide(const std::string &, const std::string &, std::string &);
        void algorithm_cleanup();
        
        int writeChannelData(ssh_channel *);

        /* packet functions */
        bool encrypt_packet(ssh_msg & msg);
        bool decrypt_packet(ssh_msg & msg);
        
        bool sending_packet() {return sendState.state != STATE_NO_PACKET;}
        bool reading_packet() {return readState.state != STATE_NO_PACKET;}

        /* returns the error string for the last error */
        const char * error_string() {return m_errorStr.c_str();}
        int getLastError() {return m_error;}

        // sends a disconnect message and then disconnects
        void disconnect(const wchar_t *, uint32);
        // disconnects without sending a disconnect message.
        void force_disconnect();

        int parse(ssh_channel *, uint32);
        int parse_extended(ssh::ssh_channel * channel, uint32 numbytes, uint32 type);

        // returns the number of seconds since the last keyexchange was performed
        uint32 timeSinceKeyExchange();
        // returns the message type of the read packet
        unsigned char getMessageType();
    protected:
        
        bool check_fingerprint(const hostkey_base *);
        void setAlgorithms(Algorithms &, Algorithms &);
        /* sets the last error */
        void setLastError(int code, const char * reason);
        /* fills a buffer with random characters */
        void randomize_buffer(unsigned char *, uint32 len);
        /* derives the keys */
        int derive_keys(kex_base * kex, Algorithms & client, Algorithms & server, const exchange_data *);
        /* derives a single key */
        int derive_key(const exchange_data *, char, unsigned char *, uint32, kex_base *);
        
        // initalizes the algorithms to use.
        int initalize_algorithms(algorithm_list &, Algorithms &, Algorithms &,kex_base **, hostkey_base **);
        bool calculate_digest(ssh_msg *, hmac_base *, uint32 seq, unsigned char * out, uint32);
        
        Algorithms clientDirection, serverDirection;    // keeps track of the used algorithms
        
        /* state information about the current recv/send */
        ConnectionState readState, sendState;

        /* some important data */
        ssh_context server_context, local_context;
        endpoint_info serverInfo;

        /* The session identifier */
        std::vector<byte> sessionIdentifier;
        
        uint32 session_length;

        SocketLayer socketLayer;

        /* the error string */
        std::string m_errorStr, m_host;
        /* the error code */
        int m_error;
        /* The last time the keyexchange was performed */
        time_t m_lastKeyExchange;
        // the saved headers
        ssh::ssh_hdr m_lastSentHdr, m_lastReadHdr;
        // the host cache
        HostCache m_knownHosts;
        ssh_notify * m_notify;
    };
};
#endif