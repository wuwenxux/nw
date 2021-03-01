
#include "nw_cli.h"
#include "utils.h"
#define K  1024
#define M 1024*1024
/*nw usage*/
void nw_peer_usage(void)
{
	fprintf(stderr,"Usage: ...peerid PEERID peerip PEERIP peerport PEERPORT\n"
					"PEERID: p1 - p255\n"
					"PEERIP: valid ip add\n"
					"PEERPORT: 1- 65535\n");
	exit(-1);
}
void nw_self_usage(void)
{
	fprintf(stderr,	"Usage:...ownid PEERID \n"
				 	"ownid\n");
	exit(-1);
}
void nw_mode_usage(void)
{
	fprintf(stderr, "Usage:...mode {server|client}\n");
	exit(-1);
}
void nw_connect_usage(void)
{
	fprintf(stderr,"Usage:...connect  dev DEVICE\n");
	exit(-1);
}
void  nw_usage(void)
{
	fprintf(stderr,"Usage:					\n"
			"	nw add { DEVICE |dev DEVICE}\n"
			"		   [ peerid  PEERID peerip PEERIP peerport PEERPORT ]\n"
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
void nw_dev_show_statistic(char *dev)
{
	
	int sock;
	struct ifreq req;
	int ret,i;
	uint64_t *p;
	char ifname[IFNAMSIZ];
	struct nw_dev_stat dev_stat;
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock == -1)
	{
		perror("create socket error.");
		setuid(getuid());
		return -1;
	}
	memset(&dev_stat,0,sizeof(struct nw_dev_stat));
	ret = nw_search_if(ifname);
	if(ret <0)
	{
		printf( " nw device not found.");
	}
	strcpy(req.ifr_name,ifname);
	dev_stat.head.type = NW_OPER_DEVSTAT;
	dev_stat.head.command = NW_COMM_READ;
	strcpy(req.ifr_data,&dev_stat);
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
const char *cli_ser = {
	"client",
	"server",
	NULL,
};
const char * mode_str( nw_mode_t mode)
{
	if( mode == NW_MODE_SERVER)
		return "server";
	else if (mode == NW_MODE_CLIENT)
		return "client";
	else
		return NULL;	
}
//show dev status
int nw_dev_search_peer(int argc, char **);
int nw_dev_show_peer(int argc ,char **);
int nw_dev_show_peers(int argc, char **);


//dev DEVICE
int nw_dev_connect(int argc, char **argv)
{	
	int ret;
	struct nw_peer_entry *entry = (struct nw_peer_entry*) malloc(sizeof(struct nw_peer_entry));
	memset(entry,0,sizeof(struct nw_peer_entry));
	entry->head.type = NW_OPER_PEER;
	entry->head.command = NW_COMM_PEER_CONNECT;
	char *dev = NULL;
	while(argc > 0)
	{
		if(strcmp(*argv,"help") == 0)
		{
			nw_usage();
		}
		else{
			if(strcmp(*argv,"dev") == 0 )
				NEXT_ARG();
			if(dev)
				duparg2("dev",*argv);
			if(check_ifname(*argv) )
				invarg("Invalid dev.\n",*argv);
			if(nw_search_if(*argv))
				invarg("not a running dev.\n",*argv);
			dev = *argv;
		} 		
		argc--;argv++;
	}
	if(!dev)
	{
		fprintf(stderr,"Not enough information:\"dev\" argument is required.\n");
		exit(-1);
	}
	strcpy(entry->head.devname,dev);
	if( nw_ioctl(&entry) < 0)
		return -1;
	return 0;
}

int main(int argc,char *argv[])
{
	int ret = -1;
	char *basename = NULL;
	basename = strrchr(*argv,'/');

	if(argc < 2 ) 
	{
		nw_usage();
	}
	else
	{
		NEXT_ARG();
		if(matches(*argv,"set") == 0 )
		{
			return nw_dev_set(argc-1,argv+1);
		}
		else if(matches(*argv,"change") == 0)
		{
			return nw_peer_change(argc-1,argv+1);
		}
		else if(matches(*argv,"show") == 0 ||
				matches(*argv,"list") == 0 ||
				matches(*argv,"lst") == 0 )
		{
			return nw_dev_show(argc-1,argv+1);
		}
		else if(matches(*argv,"connect") == 0)
		{
				return nw_dev_connect(argc-1,argv+1);
		}
		else if(matches (*argv,"add") == 0)
		{
			return nw_peer_add(argc-1,argv+1);
		}
		else if (matches(*argv,"del") == 0)
		{
			return nw_peer_del(argc-1,argv+1);
		}
		else if(matches(*argv,"close") == 0)
		{
			return nw_dev_close(argc-1,argv+1);
		}
		else if(matches(*argv,"help") == 0)
		{
			 nw_usage();
			 return 0;
		}
	}
	fprintf(stderr, "Command \"%s\" is unknown, try \"nw help\".\n",*argv);
	exit(-1);
}
