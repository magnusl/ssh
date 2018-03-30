#ifndef _ITASK_H_
#define _ITASK_H_

#include <list>

namespace ssh
{
    /* Class:           ITask
     * Description:     A task.
     */
    class ITask
    {
    public:
        enum {TASK_INIT_PENDING = 0, TASK_INIT_COMPLETE, TASK_INIT_FAILED};
        virtual int initalize()                         = 0;
        virtual bool update()                           = 0;
        virtual bool stop(bool wait = false)            = 0;
        virtual bool isInitalized()                     = 0;
        virtual bool finish() {return true;}
        void link(ITask * task) {m_links.push_back(task);}
    protected:
        std::list<ITask *> m_links;
    };
};

#endif