#ifndef _INOTIFY_H_
#define _INOTIFY_H_

namespace ssh
{
    /* Class:           INotify
     * Description:     Used to notify a component about a specific event.
     */
    class INotify
    {
    public:
        virtual int notify(uint32, uint32, void *) = 0;
    };
};

#endif