#include "utils.h"
#include "nw_cli.h"


const char *other_str[] =
{
	"Bufflen",
	"Maxbufflen",
	"queuelen",
	"oneclient",
	"showlog",
	"batch",
	"idletimeout",
	NULL,
};
static int nw_other_ioctl(struct nw_other *p)
{
    
    return 0;
}

//bufflen 256K 
int nw_other_bufflen(const char *dev, int bufflen)
{
	
	int ret;
	u32 blen= 0, maxbufflen = 0;
	struct nw_other nw_info;
	memset(&nw_info,0,sizeof(nw_info));
	
	nw_info.head.command = NW_OPER_OTHER;
	nw_info.head.type = NW_COMM_SET;
	strcpy(nw_info.head.devname,dev);
	if(get_unsigned(&nw_info.bufflen,bufflen,0));
		printf("Invalid \"%s\" value\n", bufflen);

	ret = nw_other_ioctl(&nw_info);
	if(ret == -1)
	{
		printf("other max & bufflen set error.\n");
	}
	return ret;
}
int nw_other_set(int argc, char **argv)
{
    char *dev = NULL;
    u32 maxbuflen;
    u32 queuelen;
    bool oneclient = false;
    u32 batch;
    u32 idletimeout;
    struct nw_other info;
    memset(&info,0,sizeof(struct nw_other));

}

//oneclinet yes/no
int nw_other_show(int argc,char **argv)
{
    return 0;
}