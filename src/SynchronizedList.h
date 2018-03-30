#ifndef _SYNCHRONIZEDLIST_H_
#define _SYNCHRONIZEDLIST_H_

#include <list>
#include "ThreadLock.h"
#include <assert.h>

namespace ssh
{
    /* Class:           SynchronizedList
     * Description:     A synchronized list, wraps a std::list instance.
     */
    template<class T>
    class SynchronizedList 
    {
    public:

        typedef typename std::list<T>::iterator ListIterator;

        // Inserts a element into the list
        void insert(T elem)
        {
            m_lock.lock();
            m_elements.push_back(elem);
            m_lock.unlock();
        }

        bool pop_first(T & ref)
        {
            m_lock.lock();
            if(m_elements.empty()) {
                m_lock.unlock();
                return false;
            }
            ref = m_elements.front();
            m_elements.pop_front();
            m_lock.unlock();
            return true;
        }

        // returns the number of elements in the list
        size_t size()
        {
            m_lock.lock();
            size_t _size = m_elements.size();
            m_lock.unlock();
            return _size;
        }

        bool empty()
        {
            m_lock.lock();
            bool nret = (m_elements.size() == 0);
            m_lock.unlock();
            return nret;
        }
    
        // Pops the first element of the list.
        T pop()
        {
            m_lock.lock();
            assert(m_elements.empty() == false);
            T elem = m_elements.front();
            m_elements.pop_front();
            m_lock.unlock();
            return elem;
        }

        // Removes all the elements that matches a functor.
        template<class CompareClass>
        void remove(CompareClass compare)
        {
            m_lock.lock();
            for(ListIterator it = m_elements.begin(); it != m_elements.end();it++) {
                if(compare(*it) == true) {
                    // found a match
                    if(it == m_elements.end()) {
                        // remove it
                        m_elements.erase(it);
                        break;
                    } else {
                        it = m_elements.erase(it);
                    }
                }
            }
            m_lock.unlock();
        }

        // searches the list for a match.
        template<class CompareClass>
        bool find_match(CompareClass compare, T & t)
        {
            bool bret = false;
            m_lock.lock();
            for(ListIterator it = m_elements.begin(); it != m_elements.end();it++) {
                if(compare(*it) == true) {
                    t = *it;
                    bret = true;
                    break;
                }
            }
            m_lock.unlock();
            return bret;
        }

        // removes a item from the list
        void remove(const T & item)
        {
            m_lock.lock();
            for(ListIterator it = m_elements.begin(); it != m_elements.end();it++) {
                if(*it == item) {
                    // found the element
                    if(it == m_elements.back()) {
                        // last element
                        m_elements.erase(it);
                        // break the loop.
                        break;
                    } else {
                        it = m_elements.erase(it);
                    }
                }
            }
            m_lock.unlock();
        }
    
        // applies the functor to all the elements in the list.
        template<class functor_class>
        void foreach(functor_class functor)
        {
            m_lock.lock();
            for(ListIterator it = m_elements.begin(); it != m_elements.end();it++) {
                functor(*it);
            }
            m_lock.unlock();
        }

    private:
        std::list<T>    m_elements;
        ThreadLock      m_lock;
    };
};

#endif