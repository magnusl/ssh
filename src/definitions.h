#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#define MAX_BLOCK_SIZE                      64
#define MAX_DIGEST_LENGTH                   64

#define PACKET_COMPLETE                     0
#define PACKET_PENDING                      1
#define NO_PACKET                           2
#define PACKET_SENT                         3

#define STATE_NO_PACKET                     0
#define STATE_READING_FIRSTBLOCK            1
#define STATE_READING_PAYLOAD               2
#define STATE_READING_INTEGRITY_DIGEST      3
#define STATE_SENDING_FIRST_BLOCK           4
#define STATE_SENDING_PAYLOAD               5
#define STATE_SENDING_DIGEST                6
#define STATE_READING_PACKET                7
#define STATE_SENDING_PACKET                8
#define STATE_VERIFY_INTEGRITY              9

/* status codes */
#define STATUS_ACTION_ABORTED               2
#define VALID_SIGNATURE                     1
#define AUTHENTICATION_FAILED               1
#define STATUS_SUCCESS                      0
#define STATUS_FAILURE                      -1
#define WRONG_PACKET                        -2
#define KEYEXCHANGE_FAILED                  -3
#define KEX_PARSE_ERROR                     -4
#define ALGORITHM_MISSMATCH                 -5
#define ALGORITHM_INITALIZATION_ERROR       -6
#define DISCONNECTED                        -7
#define HOST_LOOKUP_FAILED                  -8
#define NETWORK_ERROR                       -9
#define CONNECTION_FAILED                   -10
#define UNKNOWN_ERROR                       -11
#define UNSUPPORTED_PROTOCOL_VERSION        -12
#define CONNECTION_ERROR                    -13
#define CONNECTION_CLOSED                   -14
#define INVALID_PACKET_SIZE                 -15
#define INVALID_PADDING                     -16
#define CORRUPT_PACKET                      -17
#define KEXDH_PARSE_ERROR                   -18
#define MEMORY_ALLOCATION_FAILED            -19
#define TIMEOUT                             -20
#define FATAL_ERROR                         -21
#define KEY_GENERATION_FAILED               -22
#define ALGORITHM_INITALIZATION_FAILED      -23
#define INVALID_SIGNATURE                   -24
#define REQUEST_ALREADY_IN_PROGRESS         -25
#define INVALID_ARGUMENT                    -26
#define CHANNEL_INITALIZATION_FAILED        -27
#define CHANNEL_CLOSED                      -28
#define CHANNEL_NOT_ACTIVE                  -29
#define CHANNEL_EOF                         -30
#define INVALID_PARAMETERS                  -31
#define CONNECTION_ABORTED                  -32
#define INVALID_HOSTKEY                     -33
#define CONNECTION_PENDING                  -34


#define CHANNEL_OPEN                        0

#define DEFAULT_SEND_TIMEOUT        120
#define DEFAULT_READ_TIMEOUT        120
#define MAX_SECRET_LENGTH           1024
#define MAX_NUM_COMMENTS            1000
#define MAX_PADDING_SIZE            255
#define MIN_PADDING_SIZE            4
#define MAX_PACKET_SIZE             35000
#define MAX_KEYBLOB_LENGTH          8192
#define MAX_SIGNATURE_LENGTH        4096
#define MAX_KEY_LENGTH              64
#define MAX_IV_LENGTH               64
#define MAX_DERIVATION_SIZE         64
#define MAX_DIGEST_SIZE             64
#define MAX_STRING_LENGTH           2048
#define MAX_NUMBER_LENGTH           2048
#define MAX_SSH_PACKETSIZE          35000
#define MAX_PAYLOAD_SIZE            34000
#define SEND_WINDOW_SIZE            65536
#define LOCAL_WINDOW_THRESHOLD      10000
#define MIN_WINDOW_SIZE             100

#define CARRIAGE_RETURN             0x0D
#define LINE_FEED                   0x0A


#define SHA1_KEY_LENGTH             20
#define SHA1_DIGEST_LENGTH          20

/* Notification info */
#define SSH_NOTIFY_BASE                         1000
#define SSH_NOTIFY_CONNECTED                    1001
#define SSH_NOTIFY_CONNECTION_FAILED            1002
#define SSH_NOTIFY_HOST_VERIFICATION_NEEDED     1003
#define SSH_NOTIFY_AUTH_REQUIRED                1004
#define SSH_NOTIFY_AUTH_SUCCESS                 1005
#define SSH_NOTIFY_AUTH_FAILURE                 1006
#define SSH_NOTIFY_DISCONNECTED                 1007
#define SSH_NOTIFY_CONNECTION_ERROR             1008
#define SSH_NOTIFY_HOST_VERIFIED                1009

// returns a string that describes the error.
const char * getErrorMessage(int error);

#define TRANSPORT_LAYER_GENERIC(x)      (x >= 0 && x <= 19)
#define CHANNEL_RELATED_MESSAGES(x)     (x >= 90 && x <= 127)
#define AUTHENTICATION_MESSAGE(x)       (x >= 50 && x <= 79)
#define CONNECTION_PROTOCOL_GENERIC(x)  (x >= 80 && x <= 89)
#define ALGORITHM_NEGOTIATION(x)        (x >= 20 && x <= 29)
#define KEYEXCHANGE_MESSAGE(x)          (x >= 30 && x <= 49)

// Text encoding
#define SSH_ENCODING_UTF_8      1
#define SSH_ENCODING_ANSI       2
#endif
