#ifndef _SFTP_ERRORCODES_H_
#define _SFTP_ERRORCODES_H_

#define SSH_FX_OK                            0
#define SSH_FX_EOF                           1
#define SSH_FX_NO_SUCH_FILE                  2
#define SSH_FX_PERMISSION_DENIED             3
#define SSH_FX_FAILURE                       4
#define SSH_FX_BAD_MESSAGE                   5
#define SSH_FX_NO_CONNECTION                 6
#define SSH_FX_CONNECTION_LOST               7
#define SSH_FX_OP_UNSUPPORTED                8
#define SSH_FX_INVALID_HANDLE                9
#define SSH_FX_NO_SUCH_PATH                  10
#define SSH_FX_FILE_ALREADY_EXISTS           11
#define SSH_FX_WRITE_PROTECT                 12
#define SSH_FX_NO_MEDIA                      13
#define SSH_FX_NO_SPACE_ON_FILESYSTEM        14
#define SSH_FX_QUOTA_EXCEEDED                15
#define SSH_FX_UNKNOWN_PRINCIPAL             16
#define SSH_FX_LOCK_CONFLICT                 17
#define SSH_FX_DIR_NOT_EMPTY                 18
#define SSH_FX_NOT_A_DIRECTORY               19
#define SSH_FX_INVALID_FILENAME              20
#define SSH_FX_LINK_LOOP                     21
#define SSH_FX_CANNOT_DELETE                 22
#define SSH_FX_INVALID_PARAMETER             23
#define SSH_FX_FILE_IS_A_DIRECTORY           24
#define SSH_FX_BYTE_RANGE_LOCK_CONFLICT      25
#define SSH_FX_BYTE_RANGE_LOCK_REFUSED       26
#define SSH_FX_DELETE_PENDING                27
#define SSH_FX_FILE_CORRUPT                  28
#define SSH_FX_OWNER_INVALID                 29
#define SSH_FX_GROUP_INVALID                 30
#define SSH_FX_NO_MATCHING_BYTE_RANGE_LOCK   31

#endif