#include "utils.h"
#include "nw_cli.h"
static nw_other_ioctl(struct nw_other *);
int nw_other_maxbufflen(const char *dev, u32 maxbuflen );
int nw_other_queuelen(const char *dev , u32 queuelen);
bool nw_other_oneclient(const char *dev, bool oneclient)
{
    return true;
};
int nw_other_batch(const char *dev,u32 batch)
{
    if(!check_ifname(dev))
        return -1;
    return 0;
}
int nw_other_idletime(const char *dev, u32 interval)
int nw_other_log(struct nw_other* other);
int nw_other_show(struct nw_other* other);