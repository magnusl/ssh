#ifndef _EVENT_H_
#define _EVENT_H_
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#error Platform not supported for the moment
#endif

#include "types.h"

namespace ssh
{
    /* Class:           Event
     * Description:     Used to signal a event.
     */
    class Event
    {
    public:
        Event(const Event &);
        Event(const wchar_t * name = NULL, Event * master = NULL);
        void operator = (const Event &);

        ~Event();
        bool signal(int code = 0);
        bool isSignaled(int * code = NULL);
        bool reset();
        bool wait(int * code = NULL,uint32 = 120);
        bool isOwner() {return bOwner;}
    private:
#ifdef _WIN32
        HANDLE m_hEvent;
#endif
        Event * m_masterEv;
        int m_code;
        bool bOwner;
    };
};

#endif