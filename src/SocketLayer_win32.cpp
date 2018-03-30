/* File:            SocketLayer_win32.cpp
 * Description:     Contains the implementatio of the Socket layer for thw MS
 *                  Windows Platform
 * Author:          Magnus Leksell
 * Copyright © 2006-2007 Magnus Leksell
 *****************************************************************************/

#ifdef _WIN32
#include "SocketLayer.h"
#include <assert.h>
#include <iostream>
#include "definitions.h"
#include <sstream>
#include "common.h"

using namespace std;

namespace ssh
{

    /* Function:        SocketLayer::connect
     * Description:     Connects to the server. The user is able to terminate the
     *                  connection attempt by signal a event.
     */
    int SocketLayer::connect(const char * server_ip, const char * port, double maxtime, Event * abortEvent)
    {
        if(server_ip == NULL || port == NULL) {
            return INVALID_PARAMETERS;
        }

        addrinfo *info_list, hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        
        cerr << "Connecting to: " << server_ip << ":" << port << endl;
        int nret = getaddrinfo(server_ip, port, &hints, &info_list);
        if(nret != 0) {
            wcerr << L"getaddrinfo() failed: "<< (const wchar_t *) gai_strerror(WSAGetLastError()) << endl;
            return HOST_LOOKUP_FAILED;
        }
        
        sock = socket(info_list->ai_family, SOCK_STREAM, IPPROTO_TCP); 
        if(sock == INVALID_SOCKET) {
            cerr << "Failed to create the socket" << endl;
            return NETWORK_ERROR;
        }

        unsigned long val = 1;
        // set non-blocking mode.
        if(ioctlsocket(sock, FIONBIO, &val) != SOCKET_ERROR) {
            FATAL_ERROR;
        }

        // connect to the server.
        nret = ::connect(sock, (SOCKADDR*) info_list->ai_addr,  (int)info_list->ai_addrlen);
        if(nret == SOCKET_ERROR)
        {
            int code = WSAGetLastError();
            if(code == WSAEWOULDBLOCK)
            {
                timeval timeout = {0,100};
                // get the current time.
                time_t start;
                time(&start);

                // Wait until we have connected, a error occurres or the user terminates
                // the connection.
                fd_set writeSet;
                fd_set exceptSet;
                
                while(true)
                {
                    FD_ZERO(&writeSet);
                    FD_ZERO(&exceptSet);
                    FD_SET(sock, &exceptSet);
                    FD_SET(sock, &writeSet);
                    cerr << "Looping" << endl;
                    // Check if the user has aborted the attempt
                    if(abortEvent && abortEvent->isSignaled()) {    
                        DebugPrint("Connection attempt aborted by the client application.")
                        return CONNECTION_ABORTED;
                    }
                    // Check if the maximum time has been exceeded.
                    if(difftime(time(0), start) >= maxtime) {
                        DebugPrint("The connection attempt timed out.")
                        return CONNECTION_ABORTED;
                    }
                    // The connect can would block, poll with select until we are connected.
                    if(select(0,0,0,&exceptSet, &timeout) > 0) {
                        DebugPrint("Could not connect")
                        return CONNECTION_FAILED;
                    }
                    if(select(0,0,&writeSet,0,&timeout) > 0) {
                        // connected to the server
                        break;
                    }
                }
            } else {
                DebugPrint("Connection attempt failed")
                return CONNECTION_FAILED;
            }
        } else if(nret != 0) {
            DebugPrint("Unknown error")
            return CONNECTION_FAILED;
        }
        return STATUS_SUCCESS;
    }

    /* Function:        SocketLayer::connect_nonblock
     * Description:     Performs a non-blocking connection attempt.
     */
    int SocketLayer::connect_nonblock(const char * server, int port)
    {
        if(sock == 0)
        {
            if(server == NULL) {
                return INVALID_PARAMETERS;
            }
            addrinfo *info_list, hints;
            memset(&hints, 0, sizeof(hints));
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            std::stringstream ss;
            ss << port;
            int nret = getaddrinfo(server, ss.str().c_str(), &hints, &info_list);
            if(nret != 0) {
                wcerr << L"getaddrinfo() failed: "<< (const wchar_t *) gai_strerror(WSAGetLastError()) << endl;
                return HOST_LOOKUP_FAILED;
            }
        
            sock = socket(info_list->ai_family, SOCK_STREAM, IPPROTO_TCP); 
            if(sock == INVALID_SOCKET) {
                return NETWORK_ERROR;
            }

            unsigned long val = 1;
            // set non-blocking mode.
            if(ioctlsocket(sock, FIONBIO, &val) != SOCKET_ERROR) {
                FATAL_ERROR;
            }

            // connect to the server.
            nret = ::connect(sock, (SOCKADDR*) info_list->ai_addr,  (int)info_list->ai_addrlen);
            if(nret == 0) {     // connected
                return STATUS_SUCCESS;
            } else if(nret == SOCKET_ERROR) {
                if(WSAGetLastError() == WSAEWOULDBLOCK) {
                    return CONNECTION_PENDING;
                } else {
                    return STATUS_FAILURE;
                }
            }
            return FATAL_ERROR;
        } else {
            fd_set set;
            FD_ZERO(&set);
            FD_SET(sock, &set);
            timeval timeout = {0,0};
            // check for writeability (established connection)
            int nret = select(0,0,&set,0,&timeout);
            if(nret > 0) return STATUS_SUCCESS;
            else if(nret == SOCKET_ERROR) return STATUS_FAILURE;
            // check for a exception
            nret = select(0,0,0,&set,&timeout);
            if(nret > 0) return CONNECTION_FAILED;
            else if(nret == SOCKET_ERROR) return STATUS_FAILURE;    
            return CONNECTION_PENDING;
        }
    }

    /* Function:        SocketLayer::writeRawLine
     * Description:     Writes a raw line to the socket, 
     *                  terminated by a carriage return and line feed
     *                  MUST be called in blocking mode!
     */
    bool SocketLayer::writeRawLine(const std::string & str)
    {
        static const char newline[2] = {CARRIAGE_RETURN, LINE_FEED};
        // first write the string.
        int nret = send(sock, str.c_str(), (int)str.length(), 0);
        if(nret != str.length()) {
            return false;
        }
        nret = send(sock, newline, 2 * sizeof(char), 0);
        if(nret != 2) {
            return false;
        }
        return true;
    }

    /* Function:        SocketLayer::readRawLine
     * Description:     Reads a raw line from the server.
     *                  MUST be called in blocking mode
     */
    bool SocketLayer::readRawLine(std::string & str, uint32 max_length)
    {
        int nret;
        uint32 totalRead = 0, state = 0;
        char c;
        do {
            if(!data_available())
                continue;

            nret = recv(sock, &c, 1, 0);
            if(nret <= 0) {
                // error
                return false;
            } 
            // Parse the peeked data
            if(!state) {
                if(c == LINE_FEED) {
                    // missing CR, accept it anyway.
                    return true;
                } else if(c == CARRIAGE_RETURN) {
                    state = CARRIAGE_RETURN;
                } else if(totalRead < max_length) {
                    str += c;
                } else {
                    // string to long
                    return false;
                }
            } else if(state == CARRIAGE_RETURN) {
                if(c != LINE_FEED) {
                    return false;
                } else {
                    // CR followed by LF
                    return true;
                }
            }
        } while(totalRead < max_length);
        return false;
    }

    /* Function:        SocketLayer::disconnect
     * Description:     Closes the socket.
     */
    void SocketLayer::disconnect()
    {
        closesocket(sock);
    }

    /* Function:        SocketLayer::read
     * Description:     Reads data from the socket.
     */
    int SocketLayer::read(unsigned char * buff, int len)
    {
        int nret = recv(sock, (char *)buff, len, 0);
        if(nret == 0) return DISCONNECTED;
        else return nret;
    }

    /* Function:        SocketLayer::write
     * Description:     Writes data to the network stream
     */
    int SocketLayer::write(const unsigned char * buff, int len)
    {
        int nret = send(sock, (const char *)buff, len, 0);
        if(nret < 0) {
            return FATAL_ERROR;
        }
        return nret;
    }

    /* Function:        SocketLayer::data_available
     * Description:     Returns true if there is any data to be read from the socket.
     */
    bool SocketLayer::data_available()
    {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(sock, &readSet);
        timeval timeout = {0,0};
        if(select(0,&readSet,0,0,&timeout) > 0) {
            return true;
        }
        return false;
    }

    /* Function:        SocketLayer::setNonBlockingMode
     * Description:     Sets the socket in nonblocking mode.
     */
    bool SocketLayer::setNonBlockingMode()
    {
        unsigned long val = 1;
        if(ioctlsocket(sock, FIONBIO, &val) != SOCKET_ERROR) {
            return true;
        }
        return false;
    }

    /* Function:        SocketLayer::bind
     * Description:     Creates and binds the socket to a port.
     */
    int SocketLayer::bind(const char * addr, const char * port)
    {
        // create the socket.
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        // create the socket.
        if(sock == INVALID_SOCKET) {
            DebugPrint("failed to create socket")
            return FATAL_ERROR;
        }
        addrinfo *info_list, hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        int nret = getaddrinfo(addr, port, &hints, &info_list);
        if(nret != 0)   // getaddrinfo failed.
        {
            DebugPrint(gai_strerror(nret));
            closesocket(sock); sock = 0;
            return nret;
        }
        // the socket must work i a non-blocking mode.
        if(setNonBlockingMode() == false) {
            DebugPrint("setNonBlockingMode() failed");
            closesocket(sock); sock = 0;
            return STATUS_FAILURE;
        }
        // now bind the socket
        if(::bind(sock, (SOCKADDR*) info_list->ai_addr,  (int)info_list->ai_addrlen) == SOCKET_ERROR) {
            nret = WSAGetLastError();
            DebugPrint("bind() failed")
            closesocket(sock); sock = 0;
            return nret;
        }
        // set the socket in a listening state.
        if(listen(sock, 10) == SOCKET_ERROR) {
            DebugPrint("listen() failed")
            closesocket(sock); sock = 0;
            return WSAGetLastError();
        }
        return STATUS_SUCCESS;
    }

    /* Function:        SocketLayer::accept_connection
     * Description:
     */
    bool SocketLayer::accept_connection(SocketLayer & socket, std::string & addr, int & port)
    {
        timeval tv = {0,0};
        fd_set set;
        FD_ZERO(&set);
        FD_SET(sock,&set);
        // check for readability.
        int nret = select(0, &set, NULL,NULL, &tv);
        char buffer[512];
        int len = sizeof(buffer);
        if(nret > 0) {
            // connection available
            sockaddr_in * info = (sockaddr_in *)buffer;
            socket.sock = accept(sock,(sockaddr *) buffer,&len);
            if(socket.sock != INVALID_SOCKET) {
                if(info->sin_family != AF_INET) {
                    DebugPrint("SocketLayer::accept_connection(): only IPv4 sockets supported at the moment")
                    closesocket(socket.sock);
                    return false;
                } else {
                    // get the connected address and the port.
                    addr = inet_ntoa(info->sin_addr);
                    port = info->sin_port;
                    // return success.
                    return true;
                }
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
};
#endif