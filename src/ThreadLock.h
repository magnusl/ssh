#ifndef _THREADLOCK_H_
#define _THREADLOCK_H_

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#error Platform not supported
#endif

namespace ssh
{
    /* Class:           ThreadLock
     * Description:     Used for mutual exclusion between threads.
     */
    class ThreadLock
    {
    public:
        ThreadLock();
        ~ThreadLock();
        void lock();
        void unlock();
    private:
#ifdef _WIN32
        /* Windows implementation. Critical Sections are faster than mutexes. 
           We only need inter-thread synchronization */
        CRITICAL_SECTION cs;
#else
#error Platform not supported
#endif
    };

};
#endif