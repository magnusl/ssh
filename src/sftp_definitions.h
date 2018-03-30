#ifndef _SFTP_DEFINITIONS_H_
#define _SFTP_DEFINITIONS_H_

#define SFTP_REQUEST_COMPLETE               1
#define SFTP_REQUEST_PENDING                2

#define SFTP_NOTIFY_PERMISSION_DENIED           1
#define SFTP_NOTIFY_GENERIC_ERROR               2
#define SFTP_NOTIFY_UNKNOWN_ERROR               3

#define SFTP_FILE_MISSING           -10
#define SFTP_PERMISSION_DENIED      -11
#define SFTP_FAILURE                -13


// Different file types.
#define SSH_FILEXFER_TYPE_REGULAR          1
#define SSH_FILEXFER_TYPE_DIRECTORY        2
#define SSH_FILEXFER_TYPE_SYMLINK          3
#define SSH_FILEXFER_TYPE_SPECIAL          4
#define SSH_FILEXFER_TYPE_UNKNOWN          5


#endif