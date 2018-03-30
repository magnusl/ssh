/* File:            WorkThread.cpp
 * Description:     
 * Author:          Magnus Leksell
 *  
 * Copyright © 2007 Magnus Leksell, all rights reserved
 *****************************************************************************/

#include <iostream>
#include <assert.h>
#include "WorkThread.h"

using namespace std;

namespace ssh
{
    
    /* Function:        WorkThread::WorkThread
     * Description:     Constructor, initalizes the work thread.
     */
    WorkThread::WorkThread(ssh_connection * con) : m_con(con)
    {
#ifdef _WIN32
        m_thread = 0;
#endif
    }

    /* Function:        WorkThread::~WorkThread
     * Description:     Destructor.
     */
    WorkThread::~WorkThread()
    {
        if(m_thread) CloseHandle(m_thread);
    }
    
    /* Function:        WorkThread::spawn
     * Description:     Spawns the thread and begins the execution of it.   
     */
    bool WorkThread::spawn()
    {
#ifdef _WIN32
        // Create the thread,
        m_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) WorkThread::ThreadFunc, this, 0, &m_id);
        if(m_thread == NULL) {
            std::cerr << "Could not create a new thread" << endl;
            return false;
        }
#else
#error Platform not supported
#endif
        return true;
    }

    /*
     *
     */
    bool WorkThread::run()
    {
        assert(m_con != NULL);
        // Now connect to the server
        if(m_con->connect() != STATUS_SUCCESS) {
            cerr << "Could not connect to the server" << endl;
            return false;
        }
        // connected, now run the ssh main loop
        m_con->run();
        // connection closed.
        return true;
    }

    /*
     *
     */
    void WorkThread::yield()
    {
#ifdef _WIN32
        Sleep(1);   // Sleep for a while, prevents the process from stealing all the cpu time.
#endif
    }



    /* Function:        WorkThread::terminate()
     * Description:     Terminates the thread directly, will probably cause memory leaks and
     *                  resource leakages.
     */
    bool WorkThread::terminate()
    {
#ifdef _WIN32
        return (TerminateThread(m_thread, -1) == 0 ? false : true);
#endif
    }

    /*
     *
     */
    bool WorkThread::wait(unsigned int timeout)
    {
        DWORD nret = WaitForSingleObject(m_thread,timeout * 1000);
        if(nret == WAIT_OBJECT_0) return true;
        else return false;
    }

    /*
     *
     */
    DWORD WorkThread::ThreadFunc(LPVOID param)
    {
        WorkThread * thread = reinterpret_cast<WorkThread *>(param);
        if(!thread->run())
            return 0xFFFFFFFF;
        return 0;
    }

    bool WorkThread::running()
    {
        DWORD code;
        GetExitCodeThread(m_thread, &code);
        if(code == STILL_ACTIVE) return true;
        return false;
    }   
};

