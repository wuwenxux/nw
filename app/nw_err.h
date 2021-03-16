#ifndef NW_ERR_H_
#define NW_ERR_H_
/*errno */
#include <errno.h>
#define CMDERR                 -1
#define SOCKERR                -2
#define MEMERR                 -3
#define DEV_NOT_FOUND          -4
#define IOCTLERR               -5
#define PEERIDERR              -6
#define PEERDELERR            -7
#define ARGUERR                -8
#define DEVERR                 -9
#define NW_IOCTL_ERR           -10        
#define NW_OTHER_READ          -11
#define NW_BIND_READ           -12
#define NW_PING_READ           -13
#define NW_TYPE_READ           -14
#define NW_OTHER_SET           -15
#define NW_BIND_SET            -16
#define NW_PING_SET            -17
#define NW_TYPE_SET            -18        
#endif