
#include "nw_cli.h"
#include "utils.h"
#include "nw_err.h"
#include "manage.h"
#define K  1024
#define M 1024*1024

static bool is_other(bool other[]);
static bool is_other_args(const char *arg);
static void other_print(struct nw_other *,bool []);
static int get_nw_mode(const char *mode);
static int yes_no(const char *msg, const char *realval);
static int cli_ser(const char *msg, const char *realval);

//show dev status
int nw_dev_search_peer(int argc, char **);
int nw_dev_show_peer(int argc ,char **);
int nw_dev_show_peers(void);

static int get_nw_mode(const char *mode)
{
	if (strcasecmp(mode, "client") == 0 )
		return NW_MODE_CLIENT;
	else if (strcasecmp(mode, "server") == 0)
		return NW_MODE_SERVER;
	else 
		return -1;
}
static int on_off(const char *msg,const char *realval)
{
	fprintf(stderr,
		"Error: argument of \"%s\" must be \"on\" or \"off\", not \"%s\"\n",
		msg, realval);
	return -1;
}
static int yes_no(const char *msg, const char *realval)
{
	fprintf(stderr,
		"Error: argument of \"%s\" must be \"yes\" or \"no\", not \"%s\"\n",
		msg, realval);
	return -1;
}
static int cli_ser(const char *msg, const char *realval)
{
	fprintf(stderr,
		"Error: argument of \"%s\" must be \"client\" or \"server\", not \"%s\"\n",
		msg, realval);
	return -1;
}
/*nw ver*/
void nw_ver(void)
{
	fprintf(stdout,"NGM version code %-20s\n",NGMWAN_GENL_VERSION_NAME);
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
void nw_set_usage(void)
{
	fprintf(stderr, "Usage:...set dev DEV  TYPE\n"
					"						bufflen     BUFFLEN\n"
				    "						maxbufflen  MAXBUFLEN\n"
				    "						queuelen    QUEUELEN\n"
				    "						idletimeout TIMEVALUE\n"
					"						oneclient   YES|NO\n"
					"						showlog     YES|NO\n"
					"						batch       BATCH\n"
					"						switchtime  SWITCHTIME\n"
					"						bindport 	PORT\n"
					"u32 BUFFLEN MAXBUFLEN QUEUELEN BATCH SWITCHTIME  \n");
	exit(-1);
}
void nw_show_usage(void)
{
	fprintf(stderr, "Usage:...show DEV  TYPE\n"
					"					bufflen     BUFFLEN\n"
				    "					maxbufflen  MAXBUFLEN\n"
				    "					queuelen    QUEUELEN\n"
				    "					idletimeout TIMEVALUE\n"
					"					oneclient   YES|NO\n"
					"					showlog     YES|NO\n"
					"					batch       BATCH\n"
					"					switchtime  SWITCHTIME\n"
					"					bindport 	PORT\n"
					"u32 BUFFLEN MAXBUFLEN QUEUELEN BATCH SWITCHTIME\n");
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
			"	nw ver"
			"	nw stat [ DEVICE | dev DEVICE ]\n"
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

const char *dev_args_str[] = 
{
	"bufflen",
	"maxbufflen",
	"queuelen",
	"oneclient",
	"showlog",
	"batch",
	"idletimeout",
	"switchtime",
	"interval",
	"timeout",
	"bindport",
	"mode",
	"multipath",
	NULL,
};
static bool is_other_args(const char *arg)
{
	int i;
	for (i = 0; other_str[i]; i++)
	{
		if(strcmp(arg,other_str[i]) == 0)
			return true;
	}
	return false;
}
static bool is_other(bool other[])
{
	for(int i = 0 ; i < sizeof(other)/sizeof(bool); i++)
		if(other[i])
			return true;
	return false;
}
static void other_print(struct nw_other *other,bool is_other[])
{
	if(is_other[0])
		fprintf(stdout,"bufflen    \t%-10d    \n",other->bufflen);
	if(is_other[1])
		fprintf(stdout,"maxbufflen \t%-10d    \n",other->maxbufflen);
	if(is_other[2])
		fprintf(stdout,"queuelen   \t%-10d    \n",other->queuelen);
	if(is_other[3])
		fprintf(stdout,"oneclient  \t%-10s    \n",strcmp(other->oneclient,"yes")==0?"yes":"no");
	if(is_other[4])
		fprintf(stdout,"showlog    \t%-10s    \n",strcmp(other->showlog,"yes")==0?"yes":"no");
	if(is_other[5])
		fprintf(stdout,"batch      \t%-10d    \n",other->batch?other->batch:0);
	if(is_other[6])
		fprintf(stdout,"idletimeout\t%-10d    \n",other->idletimeout);
	if(is_other[7])
		fprintf(stdout,"switchtime\t%-10d     \n",other->switchtime);
}
static void ping_print(struct nw_ping *ping,bool is_ping[])
{
	if(is_ping[0])
		fprintf(stdout,"interval    \t%-10u    \n",ping->interval);
	if(is_ping[1])
		fprintf(stdout,"timeout   	\t%-10u    \n",ping->timeout);
}
char ** init_peers(char *input)
{
	char **option = malloc(MAX_PEER_NUMBER*sizeof(char *));
	for(int i = 0; i < MAX_PEER_NUMBER ;i ++)
	 	option[i] = malloc(MAX_PEERNAME_LENGTH);
	return option;
}
void free_peers(char **peers)
{
	for(int i = 0 ; i < MAX_PEER_NUMBER; i++)
	{
		free(peers[i]);
	}
	free(peers);
}
static void peer_print(struct nw_peer_entry *entry,char *id)
{
	if(entry->head.type == NW_OPER_PEER )
	{
		char ipv4[20];
		int i;
		if(entry == NULL)
			return -1;
		for(i = 0 ; i < entry->count ; i++)
		{
			if(strcmp(entry->peerid[i],id) == 0)
			{
					if(inet_ntop(AF_INET,&entry->ip[i],ipv4,16) < 0)
						return -1;
					fprintf(stdout,"peerid[%d] %s peerip %s peerport %d\n",i,entry->peerid[i],ipv4,entry->port[i]);
			}
			
		}
	}
}
//all args
//nw show dev nw1
int nw_dev_show(int argc, char **argv)
{
	char *dev = NULL;
	int ret = 0,i;
	bool mptcp = false;
	bool peer_list = false;
	bool peer_id = false;
	bool r_other[8] = {false};
	bool r_type = false;
	bool r_ping[2]= {false};
	bool r_bind = false;
	bool peerid = false;
	char *cur = NULL;
	struct nw_other other;
	struct nw_bind bind;
	struct nw_ping ping;
	struct nw_type type;
	struct nw_peer_entry *entry  = calloc(1,sizeof(struct nw_peer_entry));
	if(entry == NULL)
		return MEMERR;
	memset(&type,0,sizeof(struct nw_type));
	memset(&other,0,sizeof(struct nw_other));
	memset(&ping,0,sizeof(struct nw_ping));
	memset(&bind,0,sizeof(struct nw_bind));

	while(argc > 0)
	{
		if(strcmp(*argv,"bufflen") == 0)
		{
			r_other[0] = true;
		}else if(strcmp(*argv,"maxbufflen") == 0) 
		{
			r_other[1] = true;
		}else if(strcmp(*argv,"queuelen") == 0 )
		{
			r_other[2] = true;
		}else if(strcmp(*argv,"oneclient")== 0 )
		{
			r_other[3] = true;
		}else if(strcmp(*argv,"log") == 0 )
		{
			r_other[4] = true;
		}else if(strcmp(*argv,"batch") == 0 )
		{
			r_other[5] = true;
		}else if(strcmp(*argv,"idletimeout") == 0)
		{
			r_other[6] = true;
		}else if (strcmp(*argv,"switchtime") == 0)
		{
			r_other[7] = true;
		}else if (strcmp(*argv,"bindport") == 0)
		{
			r_bind = true;
		}else if (strcmp(*argv,"mode") == 0)
		{
			r_type = true;
		}else if (strcmp(*argv,"interval") == 0 )
		{	
			r_ping[0] = true;
		}else if (strcmp(*argv,"timeout") == 0)
		{
			r_ping[1] = true;
		}else if(strcmp(*argv,"help") == 0)
		{
			nw_show_usage();
		}else if(strcmp(*argv,"peer") == 0 )
		{
			peer_list = true;
		}else if (strcmp(*argv,"peerid") == 0)
		{
			peer_id = true;

			if(!NEXT_ARG_OK() || strlen(*argv) == 0)
			{
				fprintf(stderr,"peerid is expected.\n");
				exit(EXIT_FAILURE);
			}
			NEXT_ARG();
			ret = nw_do_peer_list(dev,entry);
			for( i = 0,cur = strtok(*argv,","); cur != NULL; cur = strtok(NULL,","), i++)
			{
				peer_print(&entry->head,cur);
			}
		}
		else
		{
			if(strcmp(*argv,"dev") == 0)
				NEXT_ARG();
			if(dev)
				duparg2("dev",*argv);
			if(check_ifname(*argv))
				invarg("\"dev\" not a valid ifname",*argv);	
			dev = *argv;
		}
		argc--;
		argv++;
	}
	if(!dev)
	{
		fprintf(stderr,"Not enough information:\"dev\" argument is required.\n");
		exit(-1);
	}
	if(!is_other(r_other) && !r_bind && !r_type && !r_ping[0] && !r_ping[1] && !peerid && !peer_list  && !peer_id)
	{
		ret = nw_other_read(dev,&other);
		do_read(&other.head);
		ret = nw_bind_read(dev,&bind);
		do_read(&bind.head);
		ret = nw_ping_read(dev,&ping);
		do_read(&ping.head);
		ret = nw_type_read(dev,&type);
		do_read(&type.head);
	}
	if(is_other(r_other))
	{
		ret = nw_other_read(dev,&other);
		//do_read(&other.head);
		other_print(&other,r_other);
	}
	if(r_type)
	{
		ret = nw_type_read(dev,&type);
		do_read(&type.head);
	}
	if(r_ping[0]||r_ping[1])
	{
		ret = nw_ping_read(dev,&ping);
		ping_print(&ping,r_ping);
	}
	if(r_bind)
	{
		ret = nw_bind_read(dev,&bind);
		do_read(&bind.head);
	}
	if(peer_list )
	{
		ret = nw_do_peer_list(dev,entry);
		do_read(&entry->head);
	}
	free(entry);
	return ret;
}
int nw_dev_set(int argc, char **argv)
{
	struct nw_other other;
	struct nw_bind bind;
	struct nw_type mo;
	struct nw_ping ping;
	struct nw_self self;
	bool do_ping = false;
	bool set_mptcp = false;
	char *dev = NULL;
	int ret ;

	memset(&other,0,sizeof(struct nw_other));
	memset(&bind,0,sizeof(struct nw_bind));
	memset(&mo,0,sizeof(struct nw_type));
	memset(&ping,0,sizeof(struct nw_ping));
	memset(&self,0,sizeof(struct nw_self));

	while(argc > 0)
	{
		if(strcmp(*argv,"bufflen") == 0)//other.bufflen
		{
			NEXT_ARG();
			if( other.bufflen != 0)
				duparg("bufflen",*argv);
			if(get_unsigned(&other.bufflen,*argv,0) || other.bufflen > 1024*4 || other.bufflen < 64)
				invarg("Invalid \"bufflen\" value between [64,1024*4] expected.\n",*argv);
		}else if(matches(*argv,"maxbufflen") == 0 || matches(*argv,"maxblen") == 0)//other.maxbufflen
		{
			NEXT_ARG();
			if(get_unsigned(&other.maxbufflen,*argv,0) )
				invarg("Invalid \"maxbufflen\" value\n",*argv);
		}else if(matches(*argv,"queuelen") == 0|| matches(*argv,"qlen") == 0)//other.queuelen
		{
			NEXT_ARG();
			if(get_unsigned(&other.queuelen,*argv,0) || other.queuelen > 1000000 || other.queuelen < 1000)
				invarg("Invalid \"maxbufflen\" value\n",*argv);
		}else if(matches(*argv,"oneclient") == 0|| matches(*argv,"onecli") == 0)//other.oneclient
		{
			NEXT_ARG();
			if(strcmp(*argv,"yes") == 0 )
			{
				strncpy(other.oneclient,"yes",3);
			}else if(strcmp (*argv, "no") == 0)
			{
				strncpy(other.oneclient,"no",2);
			}else
			{
				return yes_no("oneclient",*argv);
			}	
		}else if (matches(*argv,"log") == 0) //other.showlog
		{
			NEXT_ARG();
			if(strcmp(*argv,"yes") == 0)
			{
				strcpy(other.showlog,"yes");
			}else if (strcmp(*argv,"no") == 0)
			{
				strcpy(other.showlog,"no");
			}else
			{
				return yes_no("log",*argv);
			}
		}else if (matches(*argv,"batch") == 0 || matches(*argv,"bat") == 0)//other.batch
		{
			NEXT_ARG();
			if(get_unsigned(&other.batch,*argv,0) || other.batch >200 ||other.batch < 10)
				invarg("Invalid \"batch\" value\n",*argv);
		}else if (matches(*argv,"idletimeout") == 0 || matches(*argv,"idle") == 0) //nw_ping
		{
			NEXT_ARG();
			if(get_unsigned(&other.idletimeout,*argv,0) || other.idletimeout < 30 )
				invarg("Invalid \"idletimeout\" value\n",*argv);
		}else if( matches(*argv,"mode") == 0) //nw_type
		{
			NEXT_ARG();
			if(strcmp(*argv,"client") == 0 )
			{
				mo.mode = NW_MODE_CLIENT;
			}else if (strcmp(*argv,"server") == 0)
			{
				mo.mode = NW_MODE_SERVER;
			}else
			{
				return cli_ser("mode",*argv);
			}
		}else if( matches(*argv,"bindport") == 0)   //bind.bindport
		{
			NEXT_ARG();
			if(get_unsigned(&bind.port,*argv,0))
			{
				invarg("Invalid \"bindport \" value\n",*argv);
			}
		}else if (matches(*argv,"interval") == 0) //ping.interval
		{
			NEXT_ARG();
			if(get_unsigned(&ping.interval,*argv,0))
			{
				invarg("Invalid \"interval \" value\n",*argv);
			}
		}else if(matches(*argv,"timeout") == 0)//ping.timeout
		{
			NEXT_ARG();
			if(get_unsigned(&ping.timeout,*argv,0))
				invarg("Invalid \"timeout \" value\n",*argv);
		}else if(matches(*argv,"ownid") == 0)//nw_self
		{
			NEXT_ARG();
			strcpy(self.peerid,*argv);
		}else if(matches(*argv,"switchtime") == 0)
		{
			NEXT_ARG();
			if(get_unsigned(&other.switchtime,*argv,0))
				invarg("Invalid \"switchout\" value\n",*argv);
		}else if (matches(*argv,"multipath") == 0)
		{
			NEXT_ARG();
			if(strcmp(*argv,"on") == 0)
				set_mptcp = true;
			else if (strcmp(*argv,"off") == 0)
				set_mptcp = false;
			else
				return on_off("multipath",*argv);
		}else{//head.devname
			if(strcmp(*argv,"dev") == 0 )
				NEXT_ARG();
			else if (strcmp(*argv,"help") == 0)
				nw_set_usage();
			if(dev)
			 	duparg2("dev",*argv);
			if(check_ifname(*argv))
				invarg("\"dev\" not a valid ifname",*argv);
			dev = *argv;
		}
		argc--;
		argv++;
	}
	if(!dev)
	{
		fprintf(stderr,"Not enough information:\"dev\" argument is required.\n");
		exit(-1);
	}
	if(other.batch || other.idletimeout ||other.bufflen||other.maxbufflen||other.queuelen||strlen(other.showlog)!= 0||strlen(other.oneclient) != 0 )
	{
		if(nw_other_set(dev,&other) < 0)
			return -1;
	}
	 if(bind.port > 0)
	{
		if(nw_bind_set(dev,&bind) < 0 ) 
			return -1;
	}
	if(mo.mode)
	{
		if(nw_type_set(dev,&mo) < 0) 
			return -1;
	}
	if(ping.interval < ping.timeout)
	{
		if(nw_ping_set(dev,&ping) < 0)
			return -1;
	}
	if(set_mptcp)
	{
		char mptcp_on[40];
		sprintf(mptcp_on,"ip link set %s mptcp on",dev);
		system(mptcp_on);
	}
//	fprintf(stderr,"Not enough information. \"type\" information is required.");
	return 0 ;
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
const char *cli_ser_str = {
	"client",
	"server",
	NULL,
};
const char* mode_str( u32 mode)
{
	if( mode == NW_MODE_SERVER)
		return "server";
	else if (mode == NW_MODE_CLIENT)
		return "client";
	else
		return "client";
}


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
		else if(matches(*argv,"show") == 0 )
		{
			return nw_dev_show(argc-1,argv+1);
		}
		else if(matches(*argv,"connect") == 0||matches(*argv,"conn")== 0)
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
		}else if(matches(*argv,"ver") == 0 || matches(*argv,"version")== 0)
		{
			nw_ver();
		}
	}
	fprintf(stderr, "Command \"%s\" is unknown, try \"nw help\".\n",*argv);
	exit(-1);
}
