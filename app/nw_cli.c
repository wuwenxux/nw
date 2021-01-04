
#include "nw_cli.h"
#include "utils.h"
#define NW_CLI_BUFSIZE 512
#define K  1024
#define M 1024*1024
#define CMDERR -2
/*nw usage*/
int nw_peer_usage(void);
int nw_other_usage(void);

/*nw other*/
static int nw_other_maxbufflen(int argc, char **argv);
static int nw_other_queuelen(int argc, char **argv);
static int nw_other_oneclient(int argc, char *argv);
static int nw_other_batch(int argc, char **argv);
static int nw_other_idletime(int argc, char **argv);
static int nw_other_log(int argc, char **argv);
static int nw_other_show(int argc, char **argv);
static int nw_search_if(char *);
int nw_peer_usage(void)
{
	/*nw_peer_usage*/
	printf("nw set  { DEVICE | dev DEVICE } peer add peerid peerip peerport\n");
	printf("nw show { DEVICE | dev DEVICE } peer change peerip");
	printf("nw set  { DEVICE | dev DEVICE } dev show peer  peerid\n");
	printf("nw set  { DEVICE | dev DEVICE } dev del peer  peerid\n");
	printf("nw set  { DEVICE | dev DEVICE } dev peer statistic peerid\n");
	printf("nw set  { DEVICE | dev DEVICE } dev peer connect(client only)\n");
//	printf("nw peer close peerid\n");
	return 0;
}
int nw_ping_usage()
{
	/*nw_ping_usage*/
	printf("\nnw ping interval ***  timeout  ***\n");
	return 0;
}
int nw_other_usage(void)
{
	/*nw_other_usage*/
	printf("nw set dev nw1  bufflen 256k maxbufflen 4096M.\n");
	printf("nw set dev nw1 oneclient no. \n");
	printf("nw set dev nw1 log yes.\n ");
	printf("nw show dev nw1 other\n");
	printf("nw show dev nw1 buflen\n");
}
int nw_usage(void)
{
	nw_dev_usage();
	nw_peer_usage();
	nw_other_usage();
	nw_ping_usage();
}
static int nw_ioctl(struct nw_oper_head *head)
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
		printf("peer sockert err.\n");
        return -1;
    }
    
    ret = ioctl(sock,NW_OPER,&req);
    if(ret == -1 ) 
	{
		printf("nw %s oper %d error\n",head->devname,head->command);
	}
    close(sock);
    return ret;
}
//nw del dev nw1 peerid p1  
static int nw_peer_del(int argc, char * argv[])
{
	struct nw_peer_entry peer;
	int ret,i;
	if(argc != 6)
	{
		/*command error*/
		nw_peer_usage();
		return 0;
	}else
	{
		memset(&peer,0,sizeof(struct nw_peer_entry));
		peer.head.type = NW_OPER_PEER;
		peer.head.command = NW_COMM_PEER_DEL;
		peer.count = 1;
		ret = nw_ioctl(&peer.head);
		if(ret)
			printf("peer del error.\n");
	}
	return ret;
}
//nw change dev nw1 peerid p1 peerip 192.168.2.1 peerport 82534
static int nw_peer_change(int argc, char *argv[])
{
	struct nw_peer_entry peer;
	int ret;
	char *p,*q,**err = NULL;
	if(argc != 10)
	{
		/*command error*/
		nw_peer_usage();
		ret = 0;
		return ret ;
	}else
	{
		if(strncmp(argv[4],"peerid",6) != 0 || strncmp(argv[6],"peerip",6) != 0 || strncmp(argv[8],"peerport",8) != 0)
		{	
			return CMDERR;
		}
		memset(&peer,0,sizeof(struct nw_peer_entry));	
		strncpy(peer.peerid[0], argv[5],sizeof(argv[5]));
		inet_pton(AF_INET,argv[7],&peer.ip[0]);
		peer.port[0] =  (unsigned short) strtoul(argv[9], err, 0);
		peer.count = 1;
		peer.head.type = NW_OPER_PEER;
		peer.head.command = NW_COMM_PEER_CHANGE;
		ret = nw_ioctl(&peer.head);
		if(ret)
		{
			printf("peerip %s  peerport %s",argv[7],argv[8]);

		}			
	}
	return ret;
}
//nw add dev nw1 peerid p1 peerip 192.168.2.1 peerport 82534
static int nw_peer_add(int argc, char *argv[])
{
	struct nw_peer_entry peer;
	int ret;
	char *p,*q,**err = NULL;
	if(argc != 10)
	{
		/*command error*/
		nw_peer_usage();
		return 0;
	}else
	{
		memset(&peer,0,sizeof(struct nw_peer_entry));	
		if(strncmp(argv[4],"peerid",6) != 0 ||strncmp(argv[6],"peerip",6) != 0 ||strncmp(argv[8],"peerport",8) != 0)
		{
			perror("peerip peerport  assumed.");
			return CMDERR;		
		}
		strncpy(peer.peerid[0], argv[5],sizeof(argv[5]));

		inet_pton(AF_INET,argv[7],&peer.ip[0]);
		
		peer.port[0] = (unsigned short) strtoul(argv[9], err, 0);
		peer.count = 1;
		peer.head.type = NW_OPER_PEER;
		peer.head.command = NW_COMM_PEER_ADD;
		ret = nw_ioctl(&peer.head);
		if(ret)
		{
			printf("peerip : %s ",argv[7]);
			printf("peerpot: %s ",argv[9]);
		}
	}
		return ret;
}
//nw set dev nw1 del peerid p1,p2,p3 
static int nw_peer_list_del(int argc,char *argv[])
{
	struct nw_peer_entry peer;
	int ret,i;
	char token[MAX_PEER_NUMBER][MAX_PEERNAME_LENGTH];
	char *p,
		 *save_p =NULL,
		  **err = NULL;
	memset(&peer,0,sizeof(struct nw_peer_entry));
	memset(token,0,sizeof(token));
	if(argc !=6 )
	{
		nw_peer_usage();
		ret = 0;
		return ret;
	}
	else
	{
		strcpy(peer.head.devname,argv[3]);
		/* argv[5] p1,p2,p3*/
		if(argv[5] == NULL)
		{
				printf("command err.");
		}
		else
		{	
			for(i = 0,p = argv[5];(p = strtok_r(p,",",&save_p) != NULL);p = NULL,i++)
			{
				if(i > MAX_PEER_NUMBER -1)
				{
						printf("command error.too many peers");
						return -1;
				}
				else
				{
					strcpy(token[i],p);
					strcpy(peer.peerid[i],token[i]);
				}
			}
		}
		peer.count = i;
		peer.head.type =NW_OPER_PEER;
		peer.head.command=NW_COMM_PEER_DEL;
		ret = nw_ioctl(&peer.head);
		if(ret)
		{
			printf("ioctl err.\n");
		}
	}
	return ret;
}
//dev nw1 peer  
//NW_COMM_PEER_LIST
static int nw_peer_show(int argc, char *argv[])
{	
	struct nw_peer_entry peer;
	int ret;

	if(argc < 5)
	{
		nw_peer_usage();
		return 0;
	}
	else
	{
		memset(&peer,0,sizeof(struct nw_peer_entry));
		peer.head.type = NW_OPER_PEER;
		peer.head.command = NW_COMM_PEER_LIST;
		ret = nw_ioctl(&peer.head);
		if(ret)
		{
			printf("ioctl err.");
		}	
		return ret;
	}
}
//statistic 
//nw show dev nw1   
int nw_dev_show(int argc, char *argv[])
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
	req.ifr_data = &dev_stat;
	ret = ioctl(sock,NW_OPER_DEVSTAT,&req);
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
int nw_dev_show_peer(int argc, char **argv);
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

	/*nw1 device search*/
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
//nw set dev nw1 bufflen 256K maxbufflen 4096M
int nw_other_bufflen(const char *devname ,const char *bufflen)
{
	
	int ret;
	u32 blen= 0, maxbufflen = 0;
	struct nw_other nw_info;
	memset(&nw_info,0,sizeof(nw_info));
	
	nw_info.head.command = NW_OPER_OTHER;
	nw_info.head.type = NW_COMM_SET;
	strcpy(nw_info.head.devname,devname);
	if(get_unsigned(&nw_info.bufflen,bufflen,0));
		printf("Invalid \"%s\" value\n", bufflen);

	ret = nw_ioctl(&nw_info.head);
	if(ret == -1)
	{
		printf("other max & bufflen set error.\n");
	}
	return ret;
}
int nw_dev_usage()
{
	printf("nw set dev nw1\n");
	printf("nw show dev nw1\n");
	printf("nw show dev \n");
	return 0;
}
int nw_dev_connect(int argc, char **argv)
{
	char *dev =NULL;
	int ret;
	struct nw_peer_entry entry;
	entry.head.command = NW_OPER_PEER;
	entry.head.type = NW_COMM_PEER_CONNECT;
	ret = nw_ioctl(&entry.head);
	if(ret == -1 )
	{
		printf("nw_dev_connect %d failed",ret);
	}
	return ret;
}
//nw set dev name add peer ip port peerid
 int nw_dev_set(int argc, char **argv)
{	
	

	return 0;
}
int nw_dev_close( int argc, char **argv)
{
	return 0;
}
/*nw dev */
//dev nw1 
/*nw self*/
int nw_self_ownid(int argc, char **argv);
int do_set (int argc, char **argv);

int main(int argc,char *argv[])
{
	struct if_req ifr0;
	int ret = -1;
	u32 mask = 0;
	u32 flags = 0;
	/*other*/
	int qlen =-1;
	int bufflen =-1;
	int maxbufflen = -1;
	char dev[IF_NAMESIZE] = {0};
	struct nw_peer_entry peer_entry;
	memset(&peer_entry,0,sizeof(struct nw_peer_entry));

	while(argc > 1)
	{
		NEXT_ARG();
		if(matches(*argv,"set") == 0 || matches(*argv,"change") == 0)
		{
			return nw_dev_set(argc-1,argv+1);
		}
		else if(matches(*argv,"show") == 0)
		{
			return nw_dev_show(argc-1,argv+1);
		}
		else if(matches(*argv,"connect") == 0)
		{
			return nw_dev_connect(argc-1,argv+1);
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
}
