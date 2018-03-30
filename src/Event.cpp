#include "Event.h"
#include <assert.h>
#include "types.h"
#include <iostream>

namespace ssh
{
    using namespace std;

    // copy constructor
    Event::Event(const Event & rhs)
    {
        cerr << "Event::Event(const Event & rhs)" << endl;
        m_hEvent = rhs.m_hEvent;
        this->m_masterEv = rhs.m_masterEv;
    }

    /*
     *
     */
    Event::Event(const wchar_t * name, Event * master)
    {
#ifdef _WIN32
        m_hEvent = CreateEventW(NULL, TRUE, FALSE, name);
        assert(m_hEvent != 0);
        bOwner = (GetLastError() != ERROR_ALREADY_EXISTS);
        cerr << "Creating event with id = " << m_hEvent << endl;
#endif
        m_masterEv = master;
    }

    /* 
     *
     */
    Event::~Event()
    {
#ifdef _WIN32
        if(m_hEvent != 0) {
            cerr << "closing event=" << m_hEvent << endl;
            CloseHandle(m_hEvent);
            m_hEvent = 0;
        }
#endif
    }

    void Event::operator = (const Event & ev)
    {
        cerr << "Event::operator = (const Event & ev)" << endl;
        m_hEvent = ev.m_hEvent;
        m_masterEv = ev.m_masterEv;
    }

    /* Function:        Event::isSignaled
     * Description:     
     */
    bool Event::isSignaled(int * code)
    {
#ifdef _WIN32
        DWORD nret = WaitForSingleObject(m_hEvent, 0);
        if(code != NULL) *code = m_code;
        return (nret == WAIT_OBJECT_0);
#endif
    }

    /*
     *
     */
    bool Event::signal(int code)
    {
#ifdef _WIN32
        if(m_hEvent == NULL) return false;
        m_code = code;
        return (SetEvent(m_hEvent) != 0);
#endif
    }

    /* 
     *
     */
    bool Event::reset()
    {
#ifdef _WIN32
        if(m_hEvent == NULL) return false;
        m_code = 0;
        return (ResetEvent(m_hEvent) != 0);
#endif
    }

    /* Function:        Event::wait
     * Description:     Waits for the event to be signaled.
     */
    bool Event::wait(int * code, uint32 seconds)
    {
        HANDLE handles[2];
        DWORD count = (m_masterEv ? 2 : 1);
        handles[0] = m_hEvent;
        handles[1] = (m_masterEv ? m_masterEv->m_hEvent : 0);
        // wait for any of the objects to signal
        DWORD nret = WaitForMultipleObjects(count, handles, FALSE, seconds * 1000);
        if(nret >= WAIT_OBJECT_0 && (nret <= (WAIT_OBJECT_0 + count - 1))) {
            if(code != NULL) *code = m_code;
            return true;
        }
        else if(nret >= WAIT_ABANDONED_0 && (nret <= (WAIT_ABANDONED_0 + count - 1))) {
            return false;
        }
        return true;
    }
};