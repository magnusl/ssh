/* File:            ThreadLock_Win32.cpp
 * Description:     Windows (Win32) implementation of the ThreadLock class,
 *                  uses windows critical sections internally.
 * Author:          Magnus Leksell
 *
 * Copyright © 2006-2007 Magnus Leksell, all rights reserved.
 *****************************************************************************/

#if defined(_WIN32)
#include "ThreadLock.h"

namespace ssh
{
    /* Function:        ThreadLock::ThreadLock
     * Description:     Initalizes the lock
     */
    ThreadLock::ThreadLock()
    {
        InitializeCriticalSection(&cs);
    }

    /* Function:        ThreadLock::~ThreadLock
     * Description:     Destructor, performs the required cleanup.
     */
    ThreadLock::~ThreadLock()
    {
        DeleteCriticalSection(&cs);
    }

    /* Function:        ThreadLock::lock()
     * Description:     Acquires the lock
     */
    void ThreadLock::lock()
    {
        EnterCriticalSection(&cs);
    }

    /* Function:        ThreadLock::unlock
     * Description:     Releases the lock.
     */
    void ThreadLock::unlock()
    {
        LeaveCriticalSection(&cs);
    }

};
#endif