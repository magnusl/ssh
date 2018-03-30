#include "TaskSSH.h"
#include <iostream>

namespace ssh
{
    /* Function:        TaskSSH::TaskSSH
     * Description:     Performs the required initalization
     */
    TaskSSH::TaskSSH(ssh::ssh_connection * con, const char *addr, const char *port) 
        : m_con(con), m_addr(addr), m_port(port), m_state(0)
    {
    }

    /* Function:        TaskSSH::~TaskSSH
     * Description:     Performs the required cleanup
     */
    TaskSSH::~TaskSSH()
    {

    }

    /* Function:        TaskSSH::initalize
     * Description:     Initalizes the task, connects to the server and performs 
     *                  the keyexchange in this case.
     */
    int TaskSSH::initalize()
    {
        std::cerr << "Connecting to the server" << std::endl;
        if(m_con == NULL) return TASK_INIT_FAILED;
        if(m_con->connect(m_addr.c_str(), m_port.c_str()) != STATUS_SUCCESS) {
            std::cerr << "Could not connect to the server" << std::endl;
            return TASK_INIT_FAILED;
        }
        return TASK_INIT_COMPLETE;
    }

    /* Function:        TaskSSH::update()
     * Description:     Updates the connection.
     */
    bool TaskSSH::update()
    {
        //std::cout << "TaskSSH::update" << std::endl;
        if(m_state == TASK_INIT_COMPLETE) {
            if(m_con->update() != STATUS_SUCCESS)
                return false;
            return true;
        } else {
            if(initalize() == TASK_INIT_COMPLETE) {
                std::cerr << "Connected to the server" << std::endl;
                m_state = TASK_INIT_COMPLETE;
            } else {
                return false;
            }
        }
        return true;
    }

    /*
     *
     */
    bool TaskSSH::stop(bool wait)
    {
        return true;
    }

    /*
     *
     */
    bool TaskSSH::isInitalized()
    {
        return false;
    }

    /*
     *
     */
    bool TaskSSH::finish()
    {
        return (m_con->shutdown() == STATUS_SUCCESS);
    }
};