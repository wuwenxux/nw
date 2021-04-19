#ifndef NW_ERR_H_
#define NW_ERR_H_
/*errno */
#include <errno.h>
/**
 * Error handle
 * Different kinds of errors to return.
 * 
 **/
#define CMDERR                 -1
#define SOCKERR                -2
#define MEMERR                 -3
#define DEV_NOT_FOUND          -4
#define IOCTLERR               -5
#define PEERIDERR              -6
#define PEERDELERR             -7
#define ARGUERR                -8
#define DEV_IS_RUNNING         -9
#define NW_IOCTL_ERR           -10        
#define NW_OTHER_READ          -11
#define NW_BIND_READ           -12
#define NW_PING_READ           -13
#define NW_TYPE_READ           -14
#define NW_OTHER_SET           -15
#define NW_BIND_SET            -16
#define NW_PING_SET            -17
#define NW_TYPE_SET            -18 
#define IPV4ERR                -19 
#define FILE_NOT_FOUND         -20
#define PIPERR                 -21
#define CHECKERR               -22
#define INVALID_ARG            -23


/*nw ioctl
#define NW_OTHER_CMD_ERR    "nw_set_cmd:"
#define NW_COMM_SET         "set"
#define NW_COMM_READ        "read"

#define NW_IOCTL            "ioctl"
#define NW_PEER_CMD_ERR     "nw_peer_cmd:"
#define NW_PEER_ADD         "add"
#define NW_PEER_DEL         "del"
#define NW_PEER_LIST        "show"
#define NW_PEER_CHANGE      "change"
#define NW_SOCK             "sock"
*/




#endif