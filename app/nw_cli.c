
#include "nw_cli.h"
#include "utils.h"
#define K  1024
#define M 1024*1024
/*nw usage*/
void nw_peer_usage(void)
{
	fprintf(stderr,"Usage: ...peerid PEERID peerip PEERIP peerport PEERPORT\n"
					"PEERID: p1- p31\n"
					"PEERIP: valid ip add\n"
					"PEERPORT: 1- 65535\n");
	exit(-1);
}
int nw_self_usage(void)
{
	fprintf(stderr,	"Usage:...ownid PEERID \n"
				 	"ownid\n");
	exit(-1);
}
int nw_mode_usage(void)
{
	fprintf(stderr, "Usage:...mode {server|client}\n"
							  "mode\n");
	exit(-1);
}
int nw_usage(void)
{
	fprintf(stderr,"Usage:					\n"
			"	nw add { DEVICE |dev DEVICE}\n"
			"		   [ peer  PEERID PEERIP PEERPORT ]\n"
			" 	nw change { DEVICE |dev DEVICE }\n "
			"		   [ peer PEERID PEERIP PEERPORT ]\n "
            "	nw del { DEVICE | dev DEVICE } { peer PEERID | PEERID | PEERSID }\n"
			" 	nw set { DEVICE | dev DEVICE }\n "
            "          	          			[ bindport PORTNUM ]|\n "
            "                     			[ interval INTERVAL timeout TIMEOUT ]|\n "
			"					  			[ bufflen BUFFLEN ]|\n "
			"								[ maxbufflen MAXBUFFLEN ]\n "
			"								[ queuelen QUEUELEN ]|\n "
			"								[ oneclient {yes|no} ]|\n "
			"								[ batch BATCHSIZE ]|\n "
			"								[ idletimeout TIMEINTERVAL ]|\n "
			"								[ log ]\n "
			"                     			[ ownid OWNID ]\n "
            "				 	 			[ mode { client | server } ]\n "
			"\n"
			"	nw show [ DEVICE | dev DEVICE ][status]{ PEERID | peer PEERID }\n"
			"	nw connect [ DEVICE | dev DEVICE ]\n"
			"	nw close [ DEVICE | dev DEVICE ] [ peer PEERID| PEERID ]\n");
	exit(-1);
}

static int nw_search_if(char *);

static int get_nw_mode(const char *mode)
{
	if (strcasecmp(mode, "client") == 0)
		return NW_MODE_CLIENT;
	if (strcasecmp(mode, "server") == 0)
		return NW_MODE_SERVER;
	return -1;
}
const char *dev_stat_str[]=
{
	"UP",
	"Sendpackets",
	"SendDrops",
	"SendErrors",
	"SendBytes",
	"SendSpeed",
	"RecvPackets",
	"RecvDrops",
	"RecvErrors",
	"RecvBytes",
	"RecvSpeed",
	NULL,
};
//statistic 
//nw show dev nw1   
int nw_dev_show_statistic(int argc, char *argv[])
{
	int sock;
	struct ifreq req;
	int ret,i;
	struct nw_dev_stat dev_stat;
	uint64_t *p;
	char ifname[IFNAMSIZ];
	if(argc != 1)
	{
		/*command error*/
		return -1;
	}
	memset(&dev_stat,0,sizeof(struct nw_dev_stat));
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock == -1)
	{
		perror("create socket error.");
		setuid(getuid());
		return -1;
	}
	memset(ifname,0,sizeof(ifname));
	ret = nw_search_if(ifname);
	if(ret <0)
	{
		printf( " nw device not found.");
	}
	strcpy(req.ifr_name,ifname);
	dev_stat.head.type = NW_OPER_DEVSTAT;
	dev_stat.head.command = NW_COMM_READ;
	req.ifr_data = &dev_stat;
	ret = ioctl(sock,NW_OPER,&req);
	if(ret == -1)
	{
		perror("ioctl error.");
		close(sock);
		return -1;
	}
	close(sock);
	printf("\n NW statistic (%s)\n\n",ifname);
	p = &dev_stat.recvbytes;
	for(i = 0 ; dev_stat_str[i]; i++,p++)
	{
		printf(" %20lu |%s\n", *p,dev_stat_str[i]);
	}
	printf("\n");

	return 0;
}
//show dev status
int nw_dev_search_peer(int argc, char **);
int nw_dev_show_peer(int argc ,char **);
int nw_dev_show_peers(int argc, char **);
//nw connect dev DEVICE
int nw_dev_connect(int argc, char **argv)
{
	return 0;
}

int nw_set(int argc, char **argv)
{	
	return 0;
}

int nw_self_ownid(const char *dev, char *ownid)
{
	return 0;
}
int main(int argc,char *argv[])
{
	int ret = -1;
	/*other*/
	struct nw_other info;
	memset(&info,0,sizeof(struct nw_other));
	int qlen =-1;
	int bufflen =-1;
	int maxbufflen = -1;
	char *dev = NULL;
	char *basename = NULL;
	basename = strrchr(*argv,'/');
	/*peer*/
	struct nw_peer_entry peer_entry;
	memset(&peer_entry,0,sizeof(struct nw_peer_entry));
	NEXT_ARG();
	if(argc < 1 ) 
	{
		return nw_dev_show(0,NULL);
	}
	if(matches(*argv,"set") == 0 )
	{
		return nw_dev_set(argc-1,argv+1);
	}
	else if(matches(*argv,"change") == 0)
	{
		return nw_peer_change(argc-1,argv+1);
	}
	else if(matches(*argv,"show") == 0 ||
			matches(*argv,"list" == 0) ||
			matches(*argv,"lst"))
	{
		return nw_dev_show(argc-1,argv+1);
	}
	else if(matches(*argv,"bindport" == 0))
	{
		return nw_dev_bindport(argc-1,argv+1);
	}
	else if(matches(*argv,"connect") == 0)
	{
			return nw_dev_connect(argc-1,argv+1);
	}
	else if(matches(*argv,"close") == 0)
	{
		return nw_dev_close(argc-1,argv+1);
	}

	return 0;
}
