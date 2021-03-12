
#include "nw_cli.h"
#include "utils.h"
#include "nw_err.h"
#define K  1024
#define M 1024*1024




/*nw ver*/
void nw_ver(void)
{
	fprintf(stdout,"nw version code :%s\n",NGMWAN_GENL_VERSION_NAME);
	exit(-1);
}
/*nw usage*/
void nw_peer_usage(void)
{
	fprintf(stderr,"Usage: ...peerid PEERID peerip PEERIP peerport PEERPORT\n"
					"PEERID: p1 - p255\n"
					"PEERIP: valid ip addr\n"
					"PEERPORT: 1- 65535\n");
	exit(-1);
}
void nw_show_usage(void)
{
	fprintf(stderr,"Usage:...show dev DEV\n");
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
			"								[ switchtime SWITCHTIME]\n"
            "				 	 			[ mode { client | server } ]\n "
			"\n"
			"	nw show [ DEVICE | dev DEVICE ][status]{ PEERID | peer PEERID }\n"
			"	nw connect [ DEVICE | dev DEVICE ]\n"
			"	nw close [ DEVICE | dev DEVICE ] [ peer PEERID| PEERID ]\n");
	exit(-1);
}
const char *other_str[] =
{
	"bufflen",
	"maxbufflen",
	"queuelen",
	"oneclient",
	"showlog",
	"batch",
	"idletimeout",
	"switchtime",
	NULL,
};

const char *cs_ser =
{
	"mode",
	NULL,
};
const char *ping_str = 
{
	"interval",//ping
	"timeout",
	NULL,
};
const char *bind_str = 
{
	"bindport",
	NULL,
};

//all args
//nw show dev nw1
int nw_show_dev(int argc, char **argv)
{
	char *dev = NULL;
	int sock;

	int ret,i;
	u32 *p;
	if(argc < 2)
	{
		nw_usage();
	}else
	{
		NEXT_ARG();
		if(strcmp(*argv,"dev") == 0)
			NEXT_ARG();
		dev = *argv;
	}
	if(dev)
	{
		ret = other_read(dev);
		if(ret < 0)
			return IOCTLERR;
		ret = bind_read(dev);
		if(ret < 0)
			return IOCTLERR;
		ret = ping_read(dev);
		if(ret < 0)
			return IOCTLERR;
	}
	return 0;
	struct nw_bind bind;

	struct nw_ping ping;
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock == -1)
	{
		perror("create socket error.");
		setuid(getuid());
		return SOCKERR;
	}
	memset(&bind,0,sizeof(struct nw_bind));
	memset(&ping,0,sizeof(struct nw_ping));
	ret = nw_search_if(dev);
	if(ret <0)
	{
		printf( " nw device not found.");
		return DEV_NOT_FOUND;
	}
/*	//other
	ret = ioctl(sock,NW_OPER,&other_req);
	if(ret < 0)
	{
		fprintf(stderr,"ioctl err.\n");
		return IOCTLERR;
	}
	//ping 
	strcpy(ping_req.ifr_name,ifname);
	ping.head.command = NW_COMM_READ;
	ping.head.type = NW_OPER_PING;
	strcpy(ping_req.ifr_data,&ping);
	ret = ioctl(sock,NW_OPER,&ping_req);
	if(ret < 0)
	{
		fprintf(stderr,"ioctl err.\n");
		return IOCTLERR;
	}
	
	strcpy(bind_req.ifr_name,ifname);
	bind.head.command = NW_COMM_READ;
	bind.head.type = NW_OPER_OTHER;
	strcpy(bind_req.ifr_data,&bind);
	ret = ioctl(sock,NW_OPER,&bind_req);
	if(ret == -1)
	{
		perror("other ioctl error.");
		close(sock);
		return -1;
	}

	close(sock);
	printf("\n NW args (%s)\n\n",dev);
	p = &other.bufflen;
	for(i = 0 ; other_str[i]; i++,p++)
	{
		printf(" %s |%10d\n",other_str[i],*p);
	}
	printf("\n");
	return 0;
*/
}
static int other_read(char *dev)
{
	struct ifreq other_req;
	int i = 0;
	struct nw_other other;
	memset(&other,0,sizeof(struct nw_other));
	strcpy(other.head.devname,dev);
	other.head.command = NW_COMM_READ;
	other.head.type = NW_OPER_OTHER;
	strcpy(other_req.ifr_data,&other);

	u32 *p = &other.batch;
	for(i = 0 ; other_str[i]; i++,p++)
	{
		fprintf(stdout,"%s|%10d\n",other_str[i],*p);
	}
	return 0;
}
static int bind_read(char *dev )
{
	struct ifreq ping_req;
	struct nw_other other;

	return 0;
}
static int ping_read(char *dev)
{
	return 0;
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
//nw stat dev nw1
int nw_stat_dev(int argc, char **argv)
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
	char *dev = NULL;
	struct nw_peer_entry *entry = calloc(1,sizeof(struct nw_peer_entry));
	if(entry == NULL)
	{
		fprintf(stderr,"calloc error");
		return MEMERR;
	}
	else
	{
		entry->head.type = NW_OPER_PEER;
		entry->head.command = NW_COMM_PEER_CONNECT;
	}

	while(argc > 0)
	{
		if(strcmp(*argv,"help") == 0)
		{
			nw_connect_usage();
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
	{
		free(entry);
		return -1;
	}
	free(entry);
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
			return nw_show_dev(argc-1,argv+1);
		}
		else if(matches(*argv,"connect") == 0)
		{
				return nw_dev_connect(argc-1,argv+1);
		}
		else if(matches (*argv,"add") == 0)
		{
			return nw_peer_add(argc-1,argv+1);
		}
		else if(matches(*argv,"stat") == 0)
		{
			return nw_stat_dev(argc-1,argv+1);
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
