#ifndef _WORKTHREAD_H_
#define _WORKTHREAD_H_

// Windows stuff
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "ssh_connection.h"

namespace ssh
{
    /* Class:           WorkThread
     * Description:     A work thread.
     */
    class WorkThread
    {
    public:
        WorkThread(ssh::ssh_connection *);      // constructor
        ~WorkThread();                          // destrictor

        bool spawn();
        void yield();
        bool run();
        bool terminate();
        bool wait(unsigned int timeout = 30);
        bool running();
    protected:
        
#ifdef _WIN32
        /* Windows Implementation */
        static DWORD WINAPI ThreadFunc(LPVOID);
        HANDLE m_thread;
        DWORD m_id;
#endif

        ssh::ssh_connection * m_con;
    };
};

#endif