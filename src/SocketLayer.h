#ifndef _SOCKET_LAYER_H_
#define _SOCKET_LAYER_H_

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#endif

#include "common.h"
#include <string>
#include "Event.h"
#include <ctime>

namespace ssh
{

    /* Class:           SocketLayer
     * Description:     The lowest layer, handles the raw socket communication.
     */
    class SocketLayer
    {
    public:
        // connects to a server, blocking.
        int connect(const char * server, const char * port, double timeout = 180.0, Event * ev = NULL);
        // non-blocking connection attempt.
        int connect_nonblock(const char * server, int port);
        // disconnects from the server
        void disconnect();
        // returns the number of bytes actually written.
        bool writeRawLine(const std::string &);
        bool readRawLine(std::string &,uint32);
        int write(const unsigned char *, int);
        int read(unsigned char *, int);
        // creates and binds the socket to a specific interface and port.
        int bind(const char * addr, const char * port);
        // checks if there exist data in the socketbuffer.
        bool accept_connection(SocketLayer &, std::string &, int &);

        bool hasDataToRead();
        // sets blocking mode
        bool setBlockingMode();
        // sets nonblocking mode.
        bool setNonBlockingMode();
        bool data_available();

    private:

#ifdef _WIN32
        SOCKET sock;            // windows socket
#else
        int sock;
#endif
    };

};
#endif