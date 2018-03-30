#ifndef _CHANNEL_REQUESTS_H_
#define _CHANNEL_REQUESTS_H_

#define CHANNEL_REQUEST_BASE            0xFF
#define CHANNEL_EXEC_REQUEST            CHANNEL_REQUEST_BASE + 0
#define CHANNEL_TTY_REQUEST             CHANNEL_REQUEST_BASE + 1
#define CHANNEL_X11_REQUEST             CHANNEL_REQUEST_BASE + 2
#define CHANNEL_ENV_REQUEST             CHANNEL_REQUEST_BASE + 3
#define CHANNEL_SHELL_REQUEST           CHANNEL_REQUEST_BASE + 4
#define CHANNEL_SUBSYSTEM_REQUEST       CHANNEL_REQUEST_BASE + 5


namespace ssh
{
    /* Struct:          exec_request
     * Description:     Used to request remote execution of a command
     */
    struct exec_request
    {
        char * command;
    };

    /* Struct:          tty_settings
     * Description:     Contains the required settings for a pseduo terminal
     */
    struct tty_settings
    {
        char * term_env, term_modes;
        uint32 cols, rows, width, height;
    };

    /* Struct:          env_request
     * Description:     Sets a environment variable
     */
    struct env_request
    {
        char * variable, * value;
    };

    /* Struct:          subsystem_request
     * Description:     Used to request a subsystem.
     */
    struct subsystem_request
    {
        char * name;
    };

    struct x11_settings
    {
    };

};
#endif
