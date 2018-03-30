#include "ssh_client_sync.h"
#include <memory>
#include <iostream>

namespace ssh
{
    using namespace std;

    /* Function:        ssh_client_sync::ssh_client_sync
     * Description:     Performs the required initalization.
     */
    ssh_client_sync::ssh_client_sync()
    {
        m_ssh       = NULL;
        m_worker    = NULL;
        m_code      = 0;
    }

    /* Function:        ssh_client_sync::~ssh_client_sync
     * Description:     Performs the required cleanup
     */
    ssh_client_sync::~ssh_client_sync()
    {
        delete m_ssh;
        delete m_worker;
    }

    // retrives the last error.
    int ssh_client_sync::getLastError()
    {
        int nret;
        m_lock.lock();
        nret = m_code;
        m_lock.unlock();
        return nret;
    }

    // sets the last error
    void ssh_client_sync::setLastError(int code)
    {
        m_lock.lock();
        m_code = code;
        m_lock.unlock();
    }

    /* Function:        ssh_client_sync::isActive
     * Description:     Returns true if the connection is active.
     */
    bool ssh_client_sync::isActive()
    {
        return (m_worker->running() && !m_killEv.isSignaled());
    }

    /* Function:        ssh_client_sync::OnDisconnect
     * Description:     Called when the connection is closed.
     */
    void ssh_client_sync::OnDisconnect(const wchar_t * msg)
    {
        // signal the event. Will interupt any blocking call.
        m_killEv.signal();
    }

    /* Function:        ssh_client_sync::connect
     * Description:     Connects to the server and initalizes the secure shell protocol.
     */
    int ssh_client_sync::connect(const char * addr, const char * port, const char * config)
    {   
        WSADATA wsa;
        if(WSAStartup(MAKEWORD(2,0),&wsa) != 0) {
            cerr << "WSAStartup() failed" << endl;
            return FATAL_ERROR;
        }

        // create the ssh connection
        m_ssh = new (std::nothrow) ssh_connection(addr,port,this,&m_killEv);
        if(m_ssh == NULL) {
            cerr << "ssh_client_sync::connect: failed to allocate the ssh_connection instance" << endl;
            return FATAL_ERROR;
        }
        if(!m_ssh->initalize(config)) {
            cerr << "ssh_client_sync::connect: failed to initalize the ssh connection" << endl;
            return FATAL_ERROR;
        }
        m_worker = new (std::nothrow) ssh::WorkThread(m_ssh);
        if(m_worker == NULL) {
            cerr << "ssh_client_sync::connect: failed to allocate WorkThread instance" << endl;
            return FATAL_ERROR;
        }
        // create the event. Do this here to make sure that this thread is the owner.
        ssh::Event ev(L"Local\\__ssh_connect_event__",&m_killEv);
        if(ev.isOwner() == false) {
            return FATAL_ERROR;
        }
        // spawn the event.
        if(!m_worker->spawn()) {
            cerr << "ssh_client_sync::connect: failed to spawn worker thread" << endl;
            return FATAL_ERROR;
        }
        if(!ev.wait()) {        // timed out.
            cerr << "ssh_client_sync::connect: connection attempt timed out." << endl;
            return FATAL_ERROR;
        }
        if(!ev.isSignaled()) {      // the connection was closed.
            return -1;
        }
        return getLastError();
    }

    /* Function:        ssh_client_sync::OnConnectSuccess
     * Description:     Called when and if the connection was established correctly
     */
    void ssh_client_sync::OnConnectSuccess()
    {
        // get the event.
        ssh::Event ev(L"Local\\__ssh_connect_event__");
        // should not be the owner.
        assert(ev.isOwner() == false);
        setLastError(STATUS_SUCCESS);
        ev.signal();
    }

    /* Function:        ssh_client_sync::OnConnectFailed
     * Description:     Called if the connection fails.
     */
    void ssh_client_sync::OnConnectFailed(const char *)
    {
        // get the event.
        ssh::Event ev(L"Local\\__ssh_connect_event__");
        // should not be the owner.
        assert(ev.isOwner() == false);
        setLastError(STATUS_FAILURE);
        ev.signal();
    }

    /* Function:        ssh_client_sync::request_service
     * Description:     Requests a service
     */
    int ssh_client_sync::request_service(const char * name)
    {
        if(!isActive()) {   // the connection is not active.
            return FATAL_ERROR;
        }   
        ssh::Event ev(L"Local\\__ssh_service_event__", &m_killEv);
        if(m_ssh->request_service(name) != REQUEST_PENDING) {       
            return FATAL_ERROR;
        }
        // wait for the reply.
        if(!ev.wait()) {    // timeout
            return FATAL_ERROR;
        }
        if(!ev.isSignaled()) {  // connection closed.
            return -1;
        }
        return getLastError();
    }

    /* Function:        ssh_client_sync::OnServiceAccept
     * Description:     Called if a service is accepted.
     */
    void ssh_client_sync::OnServiceAccept(const std::string & name)
    {
        ssh::Event ev(L"Local\\__ssh_service_event__");
        assert(ev.isOwner() == false);
        setLastError(STATUS_SUCCESS);
        ev.signal();
    }

    /* Function:        ssh_client_sync::authenticate
     * Description:     Performs password authentication.
     */
    int ssh_client_sync::authenticate(const wchar_t * username, const wchar_t * password)
    {
        if(!isActive()) {
        }
        // create the event
        ssh::Event ev(L"Local\\__ssh_auth_attempt__", &m_killEv);
        assert(ev.isOwner() == true);
        // request password authentication.
        if(m_ssh->password_authentication(username,password) != REQUEST_PENDING) {
            return FATAL_ERROR;
        }
        if(!ev.wait()) {        // timeout
            return FATAL_ERROR;
        }
        if(!ev.isSignaled()) {      // connection closed
            return FATAL_ERROR;
        }
        return getLastError();
    }

    /* Function:        ssh_client_sync::OnAuthSuccess
     * Description:     Called if the authentication attempt was successful.
     */
    void ssh_client_sync::OnAuthSuccess()
    {
        ssh::Event ev(L"Local\\__ssh_auth_attempt__");
        assert(ev.isOwner() == false);
        setLastError(STATUS_SUCCESS);
        ev.signal();
    }
    
    /* Function:        ssh_client_sync::OnAuthFailure
     * Description:     Called if the authentication attempts fails.
     */
    void ssh_client_sync::OnAuthFailure()
    {
        ssh::Event ev(L"Local\\__ssh_auth_attempt__");
        assert(ev.isOwner() == false);
        setLastError(STATUS_FAILURE);
        ev.signal();
    }

    /* Function:        ssh_client_sync::OnRequestSuccess
     * Description:     Called if a global request succeeds.
     */
    void ssh_client_sync::OnRequestSuccess()
    {
        ssh::Event ev(L"Local\\__ssh_global_request__");
        assert(ev.isOwner() == false);
        setLastError(STATUS_SUCCESS);
        ev.signal();
    }

    /* Function:        ssh_client_sync::OnRequestFailure
     * Description:     Called if a global request fails.
     */
    void ssh_client_sync::OnRequestFailure()
    {
        ssh::Event ev(L"Local\\__ssh_global_request__");
        assert(ev.isOwner() == false);
        setLastError(STATUS_FAILURE);
        ev.signal();
    }

    /* Function:        ssh_client_sync::shutdown
     * Description:
     */
    void ssh_client_sync::shutdown()
    {
        // tell the ssh connection that we want to shutdown.
        m_killEv.signal();
        // now wait until the thread has finished.
        if(!m_worker->wait()) m_worker->terminate();
    }
};