#ifndef _SYNCHRONIZEDCONTAINER_H_
#define _SYNCHRONIZEDCONTAINER_H_

#include <list>

namespace ssh
{
    /*
     *
     */
    template<typename T, template <typename> class container = list >
    class SynchronizedContainer
    {
    public:
        //typedef typename container<T> container;

    protected:
        container<T> m_container;
        ThreadLock m_lock;
    };
};


#endif