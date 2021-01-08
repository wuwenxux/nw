
#include "nw_cli.h"
#include "utils.h"
#define K  1024
#define M 1024*1024
#define CMDERR -2
/*nw usage*/
int nw_peer_usage(void)
{
	fprintf(stderr,"Usage: ...peerid PEERID peerip PEERIP peerport PEERPORT\n"
					"PEERID: p1- p31\n"
					"PEERIP: valid ip add\n"
					"PEERPORT: 1- 65535\n");
	exit(-1);
}
int nw_other_usage(void);
int nw_self_usage(void);
int nw_mode_usage(void);
int nw_usage(void)
{
	fprintf(stderr,"Usage:nw add { DEVICE |dev DEVICE}\n"
			"			  [peer  PEERID PEERIP PEERPORT]\n"
			" 	nw change {DEVICE |dev DEVICE}\n "
			"		      [peer PEERID PEERIP PEERPORT]\n "
			" 	nw set { DEVICE | dev DEVICE }\n "
            "                    			[ bindport PORTNUM ]\n "
            "                     			[ peer PEERID PEERIP PEERPORT ]\n "
            "                     			[ ping  PINGARGV ]\n "
			"					  			[ other [ bufflen BUFFLEN ]|\n "
			"										[ maxbufflen MAXBUFFLEN ]|\n "
			"										[ queuelen QUEUELEN ]|\n "
			"										[ oneclient {yes|no} ]|\n "
			"										[ batch BATCHSIZE ]|\n "
			"										[ idletimeout TIMEINTERVAL ]|\n "
			"										[ log ]\n "
            "                     			[ self OWNID ]\n "
            "				 	 			[ mode {client | server} ]\n "
			"\n"
			"	nw show [ DEVICE | dev DEVICE ][status]{ peer|peer PEERID }\n"
            "	nw del { DEVICE | dev DEVICE } { peer PEERID | PEERID |PEERSID}\n"
			"	nw connect [ DEVICE | dev DEVICE ]\n"
			"	nw close [ DEVICE | dev DEVICE ] [ peer PEERID| PEERID ]\n");
	exit(-1);
}

static int nw_search_if(char *);

static int get_link_mode(const char *mode)
{
	if (strcasecmp(mode, "client") == 0)
		return NW_MODE_CLIENT;
	if (strcasecmp(mode, "server") == 0)
		return NW_MODE_SERVER;
	return -1;
}
//nw show 
int nw_dev_show (int argc, char *argv[])
{
	return 0;
}
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
int nw_dev_search_peer(const char *peerid);
int nw_dev_show_peer(const char *peerid);
int nw_dev_show_peers(const char **peerlist);
static int nw_search_if(char *ifn)
{
	FILE *fp;
	char buf[512];
	char *cmdline = "/sbin/ip link";
	char *i,*n;
	if((fp = popen(cmdline,"r")) == NULL)
	{
		err(EXIT_FAILURE,"%s",cmdline);
		return -1;
	}
	
	memset(buf,0,sizeof(buf));

	/*nw%d device search*/
	while(fgets(buf,512,fp) != NULL){
		if(*buf != ' ')
		{
			i = strtok(buf,":");
			if(i == NULL)
			{
				printf("cmd error\n");
				pclose(fp);
				return -1;
			}
			n = strtok(NULL,":");
			if(n == NULL)
			{
				printf("cmd error\n");
				pclose(fp);
			}
			if(strncmp(&n[1],"nw",2) == 0)
			{
				strcpy(ifn,&n[1]);
				return 0;
			}
		}
	}
	pclose(fp);
	
	/* not exist net device */
	return -1;
}
//nw set dev nw1 other bufflen 256K 
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

	ret = nw_ioctl(&nw_info.head);
	if(ret == -1)
	{
		printf("other max & bufflen set error.\n");
	}
	return ret;
}
//nw connect dev DEVICE;
int nw_dev_connect(int argc, char **argv);
//nw close dev DEVICE;
int nw_dev_set(int argc, char **argv)
{	
	return 0;
}
int nw_dev_close(int argc,char **argv)
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
	int qlen =-1;
	int bufflen =-1;
	int maxbufflen = -1;
	char *dev = NULL;
	char *newname = NULL;
	struct nw_peer_entry peer_entry;
	memset(&peer_entry,0,sizeof(struct nw_peer_entry));

	while(argc > 1)
	{
		NEXT_ARG();
		if(matches(*argv,"set") == 0 ||
		   matches(*argv,"change") == 0)
		{
			return nw_dev_set(argc-1,argv+1);
		}
		else if(matches(*argv,"show") == 0 ||
				matches(*argv,"list" == 0) ||
				matches(*argv,"lst"))
		{
			return nw_dev_show(argc-1,argv+1);
		}
		else if(matches(*argv,"connect") == 0)
		{
			return nw_dev_connect(argc-1,argv+1);
		}
		else if(matches(*argv,"name") == 0)
		{
			NEXT_ARG();
			newname = *argv;
		}
		else if(matches(*argv,"close") == 0)
		{
			return nw_dev_close(argc-1,argv+1);
		}
		else{
			if(matches(*argv,"dev") == 0)
			{
				NEXT_ARG();
			}else if(matches(*argv,"help") == 0)
			{
				nw_usage();
			}
			if(dev)
			{
				duparg2("dev",*argv);
			}
			if(check_ifname(*argv))
			{
				invarg("%s not a valid interface name .\n",*argv);
			}
			dev = *argv;
		}
		argc--;
		argv++;
	}
	if(!dev)
	{
		fprintf(stderr,"Not enough of information: dev argument is required. \n");
		exit(-1);
	}
	return 0;
}
