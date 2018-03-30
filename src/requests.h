#ifndef _REQUESTS_H_
#define _REQUESTS_H_

#include "types.h"
#include "definitions.h"
#include "channel_requests.h"

#define SERVICE_REQUEST                     1
#define CHANNEL_DATA_REQUEST                2
#define CHANNEL_INCREASE_WINDOW_REQUEST     3
#define CHANNEL_CLOSE_REQUEST               4
#define CHANNEL_REQUEST                     5

#define REQUEST_PENDING                     10

#define BASE_AUTHENTICATION_REQUEST         20
#define PASSWORD_AUTHENTICATION_REQUEST     BASE_AUTHENTICATION_REQUEST + 1


namespace ssh
{
    /* used for password authentication */
    struct password_auth_request
    {
        char * username, * password;
    };

    /* used for service requests */
    struct service_request
    {
        char * service_name;
    };

    struct channel_request
    {
        uint32 id;
    };

    /* Class:           request_status
     * Description:     Used to provide feedback to the application about the status of a 
     *                  request.
     */
    class request_status
    {
    public:
        request_status() {status = REQUEST_PENDING;}
        void signal(uint32 code = STATUS_SUCCESS) {status = code;}
        int getStatus() {return status;}
    private:
        uint32 status;
    };

    /* Struct:          ssh_request
     * Description:     Contains the requests.
     */
    struct ssh_request
    {
        uint32 type;
        request_status * status;
        
            // request regarding the connection.
            struct __connection
            {
                union
                {
                    password_auth_request   password;
                    service_request         service;
                    channel_request         channel;
                };
            };

            // requests regarding a channel
            struct __channel
            {
                uint32 local_id, remote_id;
                union
                {
                    exec_request        exec;
                    tty_settings        tty;
                    env_request         env;
                    subsystem_request   subsystem;
                    x11_settings        x11;
                };
            };

            union
            {
                __connection connection;
                __channel   channel;
            };
        
    };


    // performs the required cleanup on a ssh request.
    void request_cleanup(ssh_request *);
};
#endif