
#include "nw_cli.h"
#include "utils.h"
#include "nw_err.h"
#include "manage.h"
#include <ctype.h>
//#include "nw_conf.h"
#define K  1024
#define M 1024*1024
#define none_of_other !is_other(r_other,sizeof(r_other)/sizeof(r_other[0])) 
#define neither_of_ping !r_ping[0] && !r_ping[1] 
#define show_all none_of_other && neither_of_ping && !r_bind && !r_type && !peerid && !peer_list  && !peer_id && !r_self &&!mptcp
#define dev_found   !check_nw_if(dev)

static bool is_other(bool other[],size_t);
static void other_print(struct nw_other *,bool [],size_t);
static int yes_no(const char *msg, const char *realval);
static int cli_ser(const char *msg, const char *realval);
static void peer_print(struct nw_peer_entry *,char *);
static int check_self(const char *);
static void ping_print(struct nw_ping *,bool[]);
static int on_off(const char *,const char *);


/*nw ver*/
void nw_ver(void)
{
	fprintf(stdout,"ngmwan manager: version %-20s\n",NGMWAN_GENL_VERSION_NAME);
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
				    "						budget  MAXBUFLEN\n"
				    "						queuelen    QUEUELEN\n"
				    "						idletimeout TIMEVALUE\n"
					"						oneclient   YES|NO\n"
					"						showlog     YES|NO\n"
					"						batch       BATCH\n"
					"						switchtime  SWITCHTIME\n"
					"						bindport 	PORT\n");
	exit(-1);
}
void nw_show_usage(void)
{
	fprintf(stderr, "Usage:...show DEV  TYPE\n"
					"					bufflen     BUFFLEN\n"
				    "					budget  MAXBUFLEN\n"
				    "					queuelen    QUEUELEN\n"
				    "					idletimeout TIMEVALUE\n"
					"					oneclient   YES|NO\n"
					"					showlog     YES|NO\n"
					"					batch       BATCH\n"
					"					switchtime  SWITCHTIME\n"
					"					bindport 	PORT\n");
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
			"	nw add    { DEVICE | dev DEVICE}"
			"	[ peerid  PEERID peerip PEERIP peerport PEERPORT ]\n"
			" 	nw change { DEVICE | dev DEVICE } "
			"	[ peerid PEERID PEERIP PEERPORT ]\n "
            "	nw del	  { DEVICE | dev DEVICE }   	{ peer PEERID | PEERSID }\n"
			" 	nw set 	  { DEVICE | dev DEVICE }\n"
			"					 	[ bindport PORTNUM ]|\n"
			"						[ interval INTERVAL timeout TIMEOUT ]|\n "
			"						[ bufflen BUFFLEN ]|\n"
			"						[ budget budget ]\n"
			"						[ queuelen QUEUELEN ]|\n"
			"						[ oneclient {yes|no} ]|\n"
			"						[ batch BATCHSIZE ]|\n"
			"						[ idletimeout TIMEINTERVAL ]|\n"
			"						[ log ]\n"
			"						[ ownid OWNID ]\n"
			"						[ switchtime SWITCHTIME]\n"
			"						[ mode { client | server } ]\n"
			"	nw load FILE\n"
			"	nw save FILE\n"
			"	nw show [ DEVICE | dev DEVICE ][status]{ PEERID | peer PEERID }\n"
			"	nw ver\n"
			"	nw stat 	[ DEVICE | dev DEVICE ]\n"
			"	nw connect	[ DEVICE | dev DEVICE ]\n"
			"	nw close	[ DEVICE | dev DEVICE ] [ peer PEERID| PEERID ]\n");
	exit(-1);
}
const char *other_str[] =
{
	"bufflen",
	"budget",
	"queuelen",
	"oneclient",
	"showlog",
	"batch",
	"idletimeout",
	"switchtime",
	NULL,
};


const char *dev_args_str[] = 
{
	"bufflen",
	"budget",
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
int main(int argc,char *argv[])
{
	char *path = DEF_CONF_FILE;
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
		else if(matches(*argv,"load") == 0)
		{
			if( NEXT_ARG_OK())
			{
				NEXT_ARG();
				if(check_filepath(*argv))
				{
					invarg("invalid file path.",*argv);
					return -1;
				}
				path = *argv;
			}
			return nw_load_conf(path);	
		}
		else if (matches(*argv,"save") == 0)
		{
			return nw_save_conf(argc-1,argv+1);
		}
		else if(matches (*argv,"add") == 0)
		{
			return nw_peer_add(argc-1,argv+1);
		}
		else if(matches(*argv,"stat") == 0)
		{
//			return nw_stat_dev(argc-1,argv+1);
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
		}else if(matches(*argv,"ver") == 0 || matches(*argv,"version")== 0 ||matches(*argv,"-ver")==0 || matches(*argv,"-v") == 0)
		{
			nw_ver();
		}
	}
	fprintf(stderr, "Command \"%s\" is unknown, try \"nw help\".\n",*argv);
	return 0;
}

//statistic 
//nw stat dev nw1
/*
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
*/

//all args
//nw show dev nw1
int nw_dev_show(int argc, char **argv)
{
	char *dev = NULL;
	char *peers = NULL;
	int ret = 0,i;
	bool mptcp = false;
	bool peer_list = false;
	bool peer_id = false;
	bool r_other[8] = {false};
	bool r_type = false;
	bool r_ping[2]= {false};
	bool r_bind = false;
	bool peerid = false;
	bool r_self = false;
	char *cur = NULL;
	struct nw_self self;
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
	memset(&self,0,sizeof(struct nw_self));
	
	while(argc > 0)
	{
		if(strcmp(*argv,"bufflen") == 0)
		{
			r_other[0] = true;
		}else if(strcmp(*argv,"budget") == 0) 
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
			if(NEXT_ARG_OK())
			{
				peer_id = true;
				NEXT_ARG();
				peers = *argv;
			}else
			{
				peer_list = true;
			}
		}else if(strcmp(*argv,"ownid") == 0)
		{
			r_self =  true;
		}
		else if(strcmp(*argv,"multipath") == 0 )
		{
			mptcp = true;
		}
		else
		{
			if(strcmp(*argv,"dev") == 0)
				NEXT_ARG();
			if(dev)
			{
				duparg2("dev",*argv);
				goto FAILED;
			}
			if(check_ifname(*argv))
			{
				invarg(" not a valid dev name.",*argv);
				goto FAILED;
			}
			if(check_nw_if(*argv))
			{
				invarg(" not a ngmwan interface.",*argv);	
				goto FAILED;
			}
			dev = *argv;
		}
		argc--;
		argv++;
	}
	if(!dev)
	{
		fprintf(stderr,"Not enough information:\"dev\" argument is required.\n");
		goto FAILED;
	}
	PUT_BAR_T(dev);
	if(show_all&& dev_found)
	{
		ret = nw_other_read(dev,&other);
		do_read(&other.head);
		ret = nw_bind_read(dev,&bind);
		do_read(&bind.head);
		ret = nw_ping_read(dev,&ping);
		do_read(&ping.head);
		ret = nw_type_read(dev,&type);
		do_read(&type.head);
		ret = nw_self_read(dev,&self);
		do_read(&self.head);
		ret = nw_mptcp(dev);
		fprintf(stdout,"multipath  \t%-10s    \n",ret== 0?"no":"yes");
	}
	if(is_other(r_other,sizeof(r_other)/sizeof(r_other[0])))
	{
		ret = nw_other_read(dev,&other);
		//do_read(&other.head);
		other_print(&other,r_other,sizeof(r_other)/sizeof(r_other[0]));
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
	if(peer_id&& strlen(peers) > 0)
	{
		ret = nw_do_peer_list(dev,entry);
		fprintf(stdout,"No.\tid\tpeerip\t\tpeerport\n");
		for( i = 0,cur = strtok(peers,","); cur != NULL; cur = strtok(NULL,","), i++)
		{
				peer_print(entry,cur);
		}
	}
	if(r_self)
	{
		ret = nw_self_read(dev,&self);
		do_read(&self.head);
	}
	if(mptcp)
	{ 
		ret = nw_mptcp(dev);
		fprintf(stdout,"multipath  \t%-10s    \n",ret== true?"yes":"no");
	}
	goto RESULT;
RESULT:
	free(entry);
	return ret;
FAILED:
	free(entry);
	return -1;
}

int nw_dev_set(int argc, char **argv)
{
	struct nw_other other;
	struct nw_bind bind;
	struct nw_type mo;
	struct nw_ping ping;
	struct nw_self self;
	bool set_mptcp = false;
	char *dev = NULL;
	char mptcp[4];
	char mptcp_cmd[40];

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
			{
				duparg("bufflen",*argv);
				return -1;
			}
			if(get_unsigned32(&other.bufflen,*argv,0) || other.bufflen > 1024*4 || other.bufflen < 64)
			{
				invarg("invalid \"bufflen\" value between [64,1024*4] expected.\n",*argv);
				return -1;
			}	
		}else if(matches(*argv,"budget") == 0 || matches(*argv,"maxblen") == 0)//other.budget
		{
			NEXT_ARG();
			if(get_unsigned32(&other.budget,*argv,0) )
			{
				invarg("invalid \"budget\" value\n",*argv);
				return -1;
			}
		}else if(matches(*argv,"queuelen") == 0|| matches(*argv,"qlen") == 0)//other.queuelen
		{
			NEXT_ARG();
			if(get_unsigned32(&other.queuelen,*argv,0) || other.queuelen > 1000000 || other.queuelen < 1000)
			{
				invarg("invalid \"budget\" value\n",*argv);
				return -1;		
			}	
		}else if(matches(*argv,"oneclient") == 0|| matches(*argv,"onecli") == 0)//other.oneclient
		{
			NEXT_ARG();
			if(strcmp(*argv,"yes") == 0 )
			{
				strncpy(other.oneclient,"yes",4);
			}else if(strcmp (*argv, "no") == 0)
			{
				strncpy(other.oneclient,"no",3);
			}else
			{
				return yes_no("oneclient",*argv);
			}	
		}else if (matches(*argv,"log") == 0) //other.showlog
		{
			NEXT_ARG();
			if(strcmp(*argv,"yes") == 0)
			{
				strncpy(other.showlog,"yes",4);
			}else if (strcmp(*argv,"no") == 0)
			{
				strncpy(other.showlog,"no",3);
			}else
			{
				return yes_no("log",*argv);
			}
		}else if (matches(*argv,"batch") == 0 || matches(*argv,"bat") == 0)//other.batch
		{
			NEXT_ARG();
			if(get_unsigned32(&other.batch,*argv,0) || other.batch >200 ||other.batch < 10)
			{
				invarg("invalid \"batch\" value\n",*argv);
				return -1;
			}
		}else if (matches(*argv,"idletimeout") == 0 || matches(*argv,"idle") == 0) //nw_ping
		{
			NEXT_ARG();
			if(get_unsigned32(&other.idletimeout,*argv,0) || other.idletimeout < 30 )
			{
				invarg("invalid \"idletimeout\" value\n",*argv);
				return -1;
			}
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
			if(get_unsigned16(&bind.port,*argv,0))
			{
				invarg("invalid \"bindport \" value\n",*argv);
				return -1;
			}
		}else if (matches(*argv,"interval") == 0) //ping.interval
		{
			NEXT_ARG();
			if(get_unsigned32(&ping.interval,*argv,0))
			{
				invarg("invalid \"interval \" value\n",*argv);
				return -1;
			}
		}else if(matches(*argv,"timeout") == 0)//ping.timeout
		{
			NEXT_ARG();
			if(get_unsigned32(&ping.timeout,*argv,0))
			{
				invarg("invalid \"timeout \" value\n",*argv);
				return -1;
			}
		}else if(matches(*argv,"ownid") == 0)//nw_self
		{
			NEXT_ARG();
			if(check_self(*argv))
			{
				invarg("invalid \"ownid \" value\n",*argv);
				return -1;
			}
			strcpy(self.peerid,*argv);
		}else if(matches(*argv,"switchtime") == 0)
		{
			NEXT_ARG();
			if(get_unsigned32(&other.switchtime,*argv,0))
			{
				invarg("invalid \"switchtime \" value",*argv);
				return -1;
			}	
		}else if (matches(*argv,"multipath") == 0)
		{
			if( NEXT_ARG_OK())
			{
				NEXT_ARG();
				set_mptcp = true;
			if(strcmp(*argv,"on") == 0)
				strncpy(mptcp,"on",3);
			else if (strcmp(*argv,"off") == 0)
				strncpy(mptcp,"off",4);
			else
				return on_off("multipath",*argv);
			}else
			{
				fprintf(stderr,"multipath value \"on\" or \"off\" is expected.\n");
				exit(EXIT_FAILURE);
			}
		}else{//head.devname
			if(strcmp(*argv,"dev") == 0 )
				NEXT_ARG();
			else if (strcmp(*argv,"help") == 0)
				nw_set_usage();
			if(dev)
			{
			 	duparg2("dev",*argv);
				return -1;
			}
			if(check_ifname(*argv))
			{
				invarg("not a valid interface name",*argv);
				return -1;
			}if(check_nw_if(*argv))
			{
				invarg("not a ngmwan interface",*argv);
				return -1;
			}	
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
	if(other.batch || other.idletimeout ||other.bufflen||other.budget||other.queuelen||strlen(other.showlog)!= 0||strlen(other.oneclient) != 0 )
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
	if(strlen(self.peerid))
	{
		if(nw_self_set(dev,&self) < 0)
			return -1;
	}
	if(set_mptcp)
	{
		sprintf(mptcp_cmd,"ip link set %s multipath %s",dev,mptcp);
		if( system(mptcp_cmd))
		{
			fprintf(stderr,"multipath exec err.");
			exit(EXIT_FAILURE);
		}
	}
	printf("Success!\n");
	return 0 ;
}

//dev DEVICE
int nw_dev_connect(int argc, char **argv)
{	
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
			{
				invarg("Invalid dev.\n",*argv);
				goto FAILED;
			}
			if(check_nw_if(*argv))
			{
				invarg("not a ngmwan dev.\n",*argv);
				goto FAILED;
			}
			dev = *argv;
		} 		
		argc--;argv++;
	}
	if(!dev)
	{
		fprintf(stderr,"Not enough information:\"dev\" argument is required.\n");
		goto FAILED;
	}
	strcpy(entry->head.devname,dev);
	if(nw_ioctl((struct nw_oper_head *)entry) < 0)
	{
		goto FAILED;
	}
	goto SUCCESS;
SUCCESS:
	free(entry);
	return 0;
FAILED:
	free(entry);
	return -1;
}
const char* mode_str( u32 mode)
{
	if( mode == NW_MODE_SERVER)
		return "server";
	else if (mode == NW_MODE_CLIENT)
		return "client";
	else
		return "client";
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
static int check_self(const char *id)
{
    if(strlen(id) > MAX_PEERNAME_LENGTH)
        return PEERIDERR;
    if (*id == '\0')
		return -1;
	while (*id) {
		if (*id == '/' || isspace(*id))
			return PEERIDERR;
		++id;
	}
	return 0;
}

static bool is_other(bool other[],size_t size)
{
	int i;
	for(i= 0 ; i < size; i++)
		if(other[i])
			return true;
	return false;
}
static void other_print(struct nw_other *other,bool is_other[],size_t size)
{
	if(is_other[0])
		fprintf(stdout,"bufflen    \t%-10d    \n",other->bufflen);
	if(is_other[1])
		fprintf(stdout,"budget \t%-10d    \n",other->budget);
	if(is_other[2])
		fprintf(stdout,"queuelen   \t%-10d    \n",other->queuelen);
	if(is_other[3])
		fprintf(stdout,"oneclient  \t%-10s    \n",strcmp(other->oneclient,"yes")==0?"yes":"no");
	if(is_other[4])
		fprintf(stdout,"log    	   \t%-10s    \n",strcmp(other->showlog,"yes")==0?"yes":"no");
	if(is_other[5])
		fprintf(stdout,"batch      \t%-10d    \n",other->batch?other->batch:0);
	if(is_other[6])
		fprintf(stdout,"idletimeout\t%-10d    \n",other->idletimeout);
	if(is_other[7])
		fprintf(stdout,"switchtime \t%-10d     \n",other->switchtime);
}
static void ping_print(struct nw_ping *ping,bool is_ping[])
{
	if(is_ping[0])
		fprintf(stdout,"interval   \t%-10d   \n",ping->interval);
	if(is_ping[1])
		fprintf(stdout,"timeout    \t%-10d   \n",ping->timeout);
}
static void peer_print(struct nw_peer_entry *entry,char *id)
{
	if(entry->head.type == NW_OPER_PEER )
	{
		char ipv4[20];
		int i;
		for(i = 0 ; i < entry->count ; i++)
		{
			if(strcmp(entry->peerid[i],id) == 0)
			{
					inet_ntop(AF_INET,&entry->ip[i],ipv4,16);
					fprintf(stdout," %-5d\t%-5s\t%-16s%-10u\n",i,entry->peerid[i],ipv4,entry->port[i]);
			}
			
		}
	}
}