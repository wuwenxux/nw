#include "manage.h"

int nw_ioctl(struct nw_oper_head *head)
{
    struct ifreq req;
    int sock,ret;

    memset(&req,0,sizeof(req));
    req.ifr_data = (void *)head;
    strcpy(req.ifr_name,head->devname);
	head->result=0;

    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(!sock)
    {
		printf("error1\n");
        return -1;
    }
    
    ret = ioctl(sock,NW_OPER,&req);
    if(ret) {
		printf("error2\n");
	}
	
	if (!head->result) {
		printf("error3\n");
	}

    close(sock);
    return ret;
}

void testping()
{
	struct nw_ping ping;
	memset(&ping,0,sizeof(struct nw_ping));
	ping.interval=1000;
	ping.timeout=20000;
	strcpy(ping.head.devname,"nw1");
	ping.head.type=NW_OPER_PING;
	ping.head.command=NW_COMM_SET;
	struct nw_oper_head *head=(struct nw_oper_head *)&ping;
	int ret=nw_ioctl(head);
	printf("ret=%d,result=%d\n",ret,head->result);
}

int main(int argc,char *argv[])
{
	testping();
	return 0;
}
