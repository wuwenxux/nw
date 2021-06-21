
#include "nw_cli.h"
#include "utils.h"
#include "nw_err.h"
#include "manage.h"
#include <ctype.h>
#define K  1024
#define M 1024*1024
#define none_of_other !is_other(r_other,sizeof(r_other)/sizeof(r_other[0])) 
#define neither_of_ping !r_ping[0] && !r_ping[1] 
#define show_all none_of_other && neither_of_ping && !r_bind && !r_type && !peerid && !peer_list  && !peer_id && !r_self &&!mptcp&&!r_dhcp
#define dev_found   !check_nw_if(dev)
#define set_other  (set_autopeer|set_budget|set_queuelen|set_oneclient|set_showlog|set_isolate|set_idletimeout|set_switchtime|set_compress|set_simpleroute)
static bool is_other(bool other[],size_t);
static void other_print(struct nw_other *,bool [],size_t);
static void peer_print(struct nw_peer_entry *,char *,int);
static void ping_print(struct nw_ping *,bool[]);


/*nw ver*/
void nw_ver(void)
{
	fprintf(stdout,"ngmwan manager: version %-20s\n",NGMWAN_GENL_VERSION_NAME);
	return;
}
/*nw usage*/
void nw_peer_usage(void)
{
	fprintf(stderr, "Usage:  ...peerid PEERID peerip PEERIP peerport PEERPORT\n"
							"	...peer|peerid PEERID PEERIP PEERPORT\n"
							"	...PEERID PEERIP PEERPORT\n"
							"	...PEERID PEERIP\n"
							"	...PEERID\n"
					"	PEERID:  64 chars\n"
					"	PEERIP:  ip addr\n"
					"	PEERPORT: 0-65535\n");
	return;
}
void nw_set_usage(void)
{
	fprintf(stderr, "Usage:...set dev DEV\n"
						"		interval VALUE timeout VALUE\n"
						"		autopeer    	yes|no\n"
						"		isolate		yes|no\n"
						"		compress	yes|no\n"
						"		simpleroute 	yes|no\n"
						"		oneclient   	yes|no\n"
						"		log   	  	yes|no\n"
					    "		budget  	VALUE\n"
				    	"		queuelen    	VALUE\n"
				   	 	"		idletimeout 	VALUE\n"
						"		switchtime  	VALUE\n"
						"		bindport 	VALUE\n");
	return;
}
void nw_show_usage(void)
{
	fprintf(stderr, "Usage:...show DEV  TYPE\n"
						"		mode 		client|server\n"
						"		autopeer    yes|no\n"
						"		isolate		no|no\n"
						"		compress	yes|no\n"
						"		simpleroute yes|no\n"
				    	"		budget  VALUE\n"
				    	"		queuelen    VALUE\n"
				    	"		idletimeout VALUE\n"
						"		oneclient   yes|no\n"
						"		showlog     yes|no\n"
						"		switchtime  VALUE\n"
						"		bindport 	VALUE\n");
	return;

}
void nw_self_usage(void)
{
	fprintf(stderr,	"Usage:...ownid PEERID \n"
				 	"ownid\n");
	return;
}
void nw_mode_usage(void)
{
	fprintf(stderr, "Usage:...mode {server|client}\n");
	return;
}
/*
void nw_connect_usage(void)
{
	fprintf(stderr,"Usage:...connect  dev DEVICE\n");
	exit(-1);
}*/
void  nw_usage(void)
{
	fprintf(stderr,"Usage:					\n"
			"	nw add    { DEVICE | dev DEVICE } "
			" [ peerid  PEERID peerip PEERIP peerport PEERPORT ] | [ PEERID PEERIP PEERPORT ] | [ PEERID PEERIP ] | [ PEERID ]\n"
			" 	nw change { DEVICE | dev DEVICE } "
			" [ peerid  PEERID peerip PEERIP peerport PEERPORT ] | [ PEERID PEERIP PEERPORT ] | [ PEERID PEERIP ] | [ PEERID ]\n"
            "	nw del	  { DEVICE | dev DEVICE }   { peer PEERID,PEERID,PEERID }\n"
			" 	nw set 	  { DEVICE | dev DEVICE }\n"
			"   				 	[ bindport PORTNUM ]|\n"
			"						[ interval INTERVAL timeout TIMEOUT ]|\n "
			"						[ autopeer    { yes | no } ]\n"
			"						[ isolate		{ yes | no } ]\n"
			"						[ compress	{ yes | no } ]\n"
			"						[ simpleroute { yes | no } ]\n"
			"						[ budget BUDGET ]\n"
			"						[ queuelen QUEUELEN ]|\n"
			"						[ oneclient { yes | no } ]|\n"
			"						[ idletimeout IDLETIMEOUT ]|\n"
			"						[ log { yes | no } ]\n"
			"						[ ownid OWNID ]\n"
			"						[ switchtime SWITCHTIME]\n"
			"						[ mode { client | server } ]\n"
			"	nw load FILE\n"
			"	nw save FILE\n"
			"	nw show [ DEVICE | dev DEVICE ][status]{ PEERID | peer PEERID }\n"
			"	nw ver\n"
			"	nw stat [ DEVICE | dev DEVICE ]\n");
	return;
}
const char *other_str[] =
{
	"budget",
	"queuelen",
	"oneclient",
	"log",
	"idletimeout",
	"switchtime",
	"autopeer",
	"simpleroute",
	"compress",
	"isolate",
	NULL,
};


const char *dev_args_str[] = 
{
	"budget",
	"queuelen",
	"oneclient",
	"showlog",
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
	char *path = NULL;
	char *save_path = NULL;	
	//int cmd;
	if(argc < 2 ) 
	{
		nw_usage();
		return -1;
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
		/*else if(matches(*argv,"connect") == 0||matches(*argv,"conn")== 0)
		{
			return nw_dev_connect(argc-1,argv+1);
		}*/
		else if(matches(*argv,"reload") == 0)
		{
			if( NEXT_ARG_OK())
			{
				NEXT_ARG();
				if(check_filepath(*argv))
				{
					fprintf(stderr,"Error: file path [ %s ] can not be open.\n",*argv);
					return -1;
				}
				path = *argv;
			}else{
				path = DEFAULT_CONF_FILE;
			}
			return nw_reload_conf(path);
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
			}else
			{
				path = DEFAULT_CONF_FILE;
			}
			return nw_load_conf(path);
		}else if (matches(*argv,"save") == 0)
		{
			if(NEXT_ARG_OK())
			{
				NEXT_ARG();
				save_path = *argv;
			}else
			{
				save_path = DEFAULT_SAVE_FILE;
			}
			return nw_save_conf(save_path);
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
		else if(matches(*argv,"help") == 0 ||matches(*argv,"--help") == 0 ||matches(*argv,"-help") == 0||matches(*argv,"--h") == 0||matches(*argv,"-h") == 0)
		{
			nw_usage();
			return -1;
		}else if(matches(*argv,"ver") == 0 || matches(*argv,"version")== 0||matches(*argv,"--version")== 0 || matches(*argv,"--v")== 0 ||matches(*argv,"-ver")==0 || matches(*argv,"-v") == 0||matches(*argv,"--v") == 0)
		{
			nw_ver();
			return -1;
		}
	}
	fprintf(stderr, "Command \"%s\" is unknown, try \"nw --help \".\n",*argv);
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
}*/

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
	bool r_other[10] = {false};
	bool r_type = false;
	bool r_ping[2]= {false};
	bool r_bind = false;
	bool peerid = false;
	bool r_self = false;
	bool r_dhcp = false;
	int idlen;
	char *tempPeers = NULL;
	char *cur = NULL;
	struct nw_self self;
	struct nw_other other;
	struct nw_bind bind;
	struct nw_ping ping;
	struct nw_type type;
	struct nw_dhcp dhcp;
	struct nw_peer_entry *entry  = calloc(1,sizeof(struct nw_peer_entry));
	if(entry == NULL)
		return MEMERR;
	memset(&type,0,sizeof(struct nw_type));
	memset(&other,0,sizeof(struct nw_other));
	memset(&ping,0,sizeof(struct nw_ping));
	memset(&bind,0,sizeof(struct nw_bind));
	memset(&self,0,sizeof(struct nw_self));
	memset(&dhcp,0,sizeof(struct nw_dhcp));

	while(argc > 0)
	{
		if(strcmp(*argv,"compress") == 0)
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
		}else if(strcmp(*argv,"idletimeout") == 0)
		{
			r_other[5] = true;
		}else if (strcmp(*argv,"switchtime") == 0)
		{
			r_other[6] = true;
		}else if(strcmp(*argv,"autopeer") == 0)
		{
			r_other[7] = true;
		}else if(strcmp(*argv,"simpleroute") == 0)
		{
			r_other[8] = true;
		}else if(strcmp(*argv,"isolate") == 0)
		{
			r_other[9] = true;
		}
		else if (strcmp(*argv,"bindport") == 0)
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
			return -1;
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
		else if(strcmp(*argv,"dhcp") == 0)
		{
			r_dhcp = true;
		}else
		{
			if(strcmp(*argv,"dev") == 0 )
			{
				if( NEXT_ARG_OK())
					NEXT_ARG();
				else 
				{
					nw_show_usage();
					return -1;
				}
			}
			if(dev)
			{
				duparg2(dev,*argv);
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
		fprintf(stderr,"Error:not enough information:\"dev\" argument is required.\n");
		goto FAILED;
	}
	PUT_BAR_T(dev);
	if(show_all&& dev_found)
	{
		ret = nw_type_read(dev,&type);
		do_read(&type.head);
		ret = nw_other_read(dev,&other);
		do_read(&other.head);
		ret = nw_bind_read(dev,&bind);
		do_read(&bind.head);
		ret = nw_ping_read(dev,&ping);
		do_read(&ping.head);
		ret = nw_self_read(dev,&self);
		do_read(&self.head);
		ret = nw_mptcp_read(dev);
		fprintf(stdout,"multipath  	\t%-10s    \n",ret == 0?"off":"on");
		ret = nw_dhcp_read(dev,&dhcp);
		if(type.mode == NW_MODE_SERVER)
		{
			do_read(&dhcp.head);
		}else
		{
			fprintf(stdout,"dhcp  	\t\t%-10s    \n",dhcp.enable);
		}
	}
	if(is_other(r_other,sizeof(r_other)/sizeof(r_other[0])))
	{
		ret = nw_other_read(dev,&other);
		other_print(&other,r_other,sizeof(r_other)/sizeof(r_other[0]));
	}
	if(r_type)
	{
		ret = nw_type_read(dev,&type);
		do_read(&type.head);
	}
	if(r_ping[0] || r_ping[1])
	{
		ret = nw_ping_read(dev,&ping);
		ping_print(&ping,r_ping);
	}
	if(r_bind)
	{
		ret = nw_bind_read(dev,&bind);
		do_read(&bind.head);
	}
	if(peer_list)
	{
		ret = nw_do_peer_list(dev,entry);
		do_read(&entry->head);
	}
	if(peer_id&& strlen(peers) > 0)
	{
		idlen = 0;
		ret = nw_do_peer_list(dev,entry);
		tempPeers = malloc(strlen(peers));
		if( tempPeers == NULL)
			return -1;
		strcpy(tempPeers,peers);
		for( i = 0,cur = strtok(tempPeers,","); cur != NULL; cur = strtok(NULL,","), i++)
		{
			idlen = idlen > strlen(cur) ? idlen : strlen(cur);
		}
		fprintf(stdout,"No. ID%-*sIP%-*s Port%-*s \n",idlen," ",15," ",5," ");	
		for( i = 0,cur = strtok(peers,","); cur != NULL; cur = strtok(NULL,","), i++)
		{
			peer_print(entry,cur,idlen);
		}
		free(tempPeers);
	}
	if(r_self)
	{
		ret = nw_self_read(dev,&self);
		do_read(&self.head);
	}
	if(mptcp)
	{ 
		ret = nw_mptcp_read(dev);
		fprintf(stdout,"multipath  \t%-10s    \n",ret== 0?"no":"yes");
	}
	if(r_dhcp)
	{
		ret = nw_dhcp_read(dev,&dhcp);
		do_read(&dhcp.head);
	}
	goto RESULT;
RESULT:
	free(entry);
	return 0;
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
	struct nw_dhcp dhcp;
	nw_set_flags flags= nw_set_nothing;
	dhcp_flags d_flag= dhcp_nothing;
	other_flags o_flag = other_nothing;
	ping_flags p_flag = ping_nothing;
	int ret;
	char *dev = NULL;
	bool mptcp= false;
	memset(&other,0,sizeof(struct nw_other));
	memset(&bind,0,sizeof(struct nw_bind));
	memset(&mo,0,sizeof(struct nw_type));
	memset(&ping,0,sizeof(struct nw_ping));
	memset(&self,0,sizeof(struct nw_self));
	memset(&dhcp,0,sizeof(struct nw_dhcp));

	if(argc == 1 || argc == 2) 
	{
		nw_set_usage();
		return -1;
	}
	while(argc > 0)
	{
		if(matches(*argv,"budget") == 0 || matches(*argv,"budg") == 0)//other.budget
		{
			flags |= nw_set_other;
			o_flag |= set_budget;
			if(NEXT_ARG_OK())
			{
				NEXT_ARG();
				if(get_unsigned32(&other.budget,*argv,0))
				{
					invarg("invalid \"budget\" value,is supposed to be divided by 64.",*argv);
					return -1;
				}
			}else
			{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}
		}else if(matches(*argv,"queuelen") == 0|| matches(*argv,"qlen") == 0)//other.queuelen
		{
			flags |= nw_set_other;
			o_flag |= set_queuelen;
			if(NEXT_ARG_OK())
			{
				NEXT_ARG();
				if(get_unsigned32(&other.queuelen,*argv,0) || other.queuelen > 1000000 || other.queuelen < 1000)
				{
					invarg("invalid \"budget\" value,is supposed to be between [1000,1000000].",*argv);
					return -1;		
				}
			}else
			{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}	
		}else if(matches(*argv,"oneclient") == 0|| matches(*argv,"onecli") == 0)//other.oneclient
		{
			flags |= nw_set_other;
			o_flag |= set_oneclient;
			if(NEXT_ARG_OK())
			{
				NEXT_ARG();
				if(strcmp(*argv,"yes") == 0 || strcmp(*argv,"no") == 0)
				{
					strcpy(other.oneclient,*argv);
				}else
				{
					return yes_no("oneclient",*argv);
				}
			}else
			{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}	
		}else if(matches(*argv,"log") == 0) //other.showlog
		{
			flags |= nw_set_other;
			o_flag |= set_showlog;
			if(NEXT_ARG_OK()){
				NEXT_ARG();
				if(strcmp(*argv,"yes") == 0 || strcmp(*argv,"no") == 0)
				{
					strcpy(other.showlog,*argv);
				}else
				{
					return yes_no("log",*argv);
				}
			}else
			{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}
		}else if(matches(*argv,"autopeer") == 0) //other.autopeer
		{
			flags |= nw_set_other;
			o_flag |= set_autopeer;
			if(NEXT_ARG_OK()){
				NEXT_ARG();
				if(strcmp(*argv,"yes") == 0 ||strcmp(*argv,"no") == 0) 
				{
					strcpy(other.autopeer,*argv);
				}else
				{
					return yes_no("autopeer",*argv);
				}
			}else
			{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}
		}else if(matches(*argv,"isolate") == 0) //other.showlog
		{
			flags |= nw_set_other;
			o_flag |= set_isolate;
			if(NEXT_ARG_OK()){
				NEXT_ARG();
				if(strcmp(*argv,"yes") == 0 || strcmp(*argv,"no") == 0)
				{
					strcpy(other.isolate,*argv);
				}else
				{
					return yes_no("isolate",*argv);
				}
			}else{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}
		}else if(matches(*argv,"compress") == 0) //other.showlog
		{
			flags |= nw_set_other;
			o_flag |= set_compress;
			if(NEXT_ARG_OK()){
				NEXT_ARG();
				if(strcmp(*argv,"yes") == 0 || strcmp(*argv,"no") == 0)
				{
					strcpy(other.compress,"no");
				}else
				{
					return yes_no("compress",*argv);
				}
			}else
			{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}
		}else if(matches(*argv,"simpleroute") == 0) //other.showlog
		{
			flags |= nw_set_other;
			o_flag |= set_simpleroute;
			if(NEXT_ARG_OK()){
				NEXT_ARG();
				if(strcmp(*argv,"yes") == 0 ||strcmp(*argv,"no") == 0)
				{
					strcpy(other.simpleroute,*argv);
				}else{
					return yes_no("simpleroute",*argv);
				}
			}else
			{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}
		}else if (matches(*argv,"idletimeout") == 0 || matches(*argv,"idle") == 0)//other.idletimeout
		{
			flags |= nw_set_other;
			o_flag |= set_idletimeout;
			if(NEXT_ARG_OK()){
				NEXT_ARG();
				if(get_unsigned32(&other.idletimeout,*argv,0) || other.idletimeout < 30 )
				{
					invarg("invalid \"idletimeout\" value,supposed to be greater than 30.",*argv);
					return -1;
				}
			}else{
				fprintf(stderr,"value of %s is not exist",*argv);
				return -1;
			}
		}else if( matches(*argv,"mode") == 0) //nw_type
		{
			flags |= nw_set_type;
			if(NEXT_ARG_OK()){
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
			}else{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1; 
			}
		}else if( matches(*argv,"bindport") == 0)   //bind.bindport
		{
			flags |= nw_set_bind;
			if(NEXT_ARG_OK())
			{
				NEXT_ARG();
				if(get_unsigned16(&bind.port,*argv,0))
				{
					invarg("invalid \"bindport \" value.",*argv);
					return -1;
				}
			}else
			{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}
		//tongshi shuru
		}else if (matches(*argv,"interval") == 0) //ping.interval
		{
			flags |= nw_set_ping;
			p_flag |= set_interval;
			if(NEXT_ARG_OK())
			{
				NEXT_ARG();
				if(get_unsigned32(&ping.interval,*argv,0))
				{
					invarg("invalid \"interval \" value.",*argv);
					return -1;
				}
			}else
			{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}
		}
		else if(matches(*argv,"timeout") == 0)//ping.timeout
		{
			flags |= nw_set_ping;
			p_flag |= set_timeout;
			if(NEXT_ARG_OK())
			{
				NEXT_ARG();
				if(get_unsigned32(&ping.timeout,*argv,0))
				{
					invarg("not a valid \"timeout \" value.",*argv);
					return -1;
				}
			}else
			{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}
		}else if(matches(*argv,"ownid") == 0)//nw_self
		{
			flags |= nw_set_self;
			if(NEXT_ARG_OK()){
				NEXT_ARG();
				if(check_self(*argv))
				{
					invarg("invalid \"ownid \" value.",*argv);
					return -1;
				}
				strcpy(self.peerid,*argv);
			}else
			{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}
		}else if(matches(*argv,"dhcp") == 0) //nw_dhcp
		{
			flags|= nw_set_dhcp;
			d_flag |= set_enable;
			if(NEXT_ARG_OK()){
				NEXT_ARG();
				if(strcmp(*argv,"yes") == 0 ||strcmp(*argv,"no") == 0)
				{
					strcpy(dhcp.enable,*argv);
				}else
				{
					return yes_no("dhcp",*argv);
				}
			}else
			{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}
		}else if(matches(*argv,"dhcp-startip") == 0)
		{
			flags  |= nw_set_dhcp;
			d_flag |= set_startip; 
			if(NEXT_ARG_OK()){
				NEXT_ARG();
				if(check_ipv4(*argv))
				{
					fprintf(stderr,"Error:startip %s is invalid.\n",*argv);
					goto param_ip_error;
				}
				inet_pton(AF_INET,*argv,&dhcp.startip);
			}else
			{
				fprintf(stderr,"Error: value of %s is not exist.\n",*argv);
				return -1;
			}
		}else if(matches(*argv,"dhcp-endip") == 0)
		{
			flags  |= nw_set_dhcp;
			d_flag |= set_endip;
			if(NEXT_ARG_OK()){
				NEXT_ARG();
				if(check_ipv4(*argv))
				{
					fprintf(stderr,"Error:%s invalid endip addr.\n",*argv);
					goto param_ip_error;
				}
				inet_pton(AF_INET,*argv,&dhcp.endip);
			}else
			{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}
		}else if(matches(*argv,"dhcp-mask") == 0)
		{
			flags  |= nw_set_dhcp;
			d_flag |= set_mask;
			if(NEXT_ARG_OK())
			{
				NEXT_ARG();
				if(check_netmask(*argv))
				{
					fprintf(stderr,"Invalid ipv4 mask.\n");
					goto param_mask_error;
				}
				inet_pton(AF_INET,*argv,&dhcp.mask);
			}else
			{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}
		}
		else if(matches(*argv,"switchtime") == 0)
		{
			flags  |= nw_set_other;
			o_flag |= set_switchtime;
			if(NEXT_ARG_OK()){
				NEXT_ARG();
				if(get_unsigned32(&other.switchtime,*argv,0))
				{
					invarg("invalid \"switchtime \" value.",*argv);
					return -1;
				}
			}else
			{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}
		}else if (matches(*argv,"multipath") == 0 || matches(*argv,"mptcp") == 0)
		{
			flags |= nw_set_mptcp;
			if( NEXT_ARG_OK())
			{
				NEXT_ARG();
				if(strcmp(*argv,"on") == 0 )
					mptcp = true;
				else if (strcmp(*argv,"off") == 0)
					mptcp = false;
				else
					return on_off("multipath",*argv);
			}else
			{
				fprintf(stderr,"value of %s is not exist.\n",*argv);
				return -1;
			}
		}else
		{//dev
			flags |= nw_set_dev;
			if(strcmp(*argv,"dev") == 0 )
			{
				if(NEXT_ARG_OK())
					NEXT_ARG();
				else {
					nw_set_usage();
					exit(EXIT_FAILURE);
				}
			}else if (strcmp(*argv,"help") == 0)
				nw_set_usage();
			if(dev)
			{
			 	duparg2(dev,*argv);
				return -1;
			}
			if(check_ifname(*argv))
			{
				invarg("not a valid interface name.",*argv);
				return -1;
			}
			if(check_nw_if(*argv))
			{
				invarg("not a running nw interface.",*argv);
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
		exit(EXIT_FAILURE);
	}
	if(flags & nw_set_other)
	{
		ret = nw_other_set(dev,&other);
		if(ret)
			goto set_failure;
	//fprintf(stdout,"other success.\n");
	}
	if(flags & nw_set_bind)
	{
		ret = nw_bind_set(dev,&bind);
		if(ret)
		{
			fprintf(stderr,"Error:bindport set failure.\n");
			goto set_failure;
		}
	}
	if(flags & nw_set_type)
	{
		ret = nw_type_set(dev,&mo); 
		if(ret)
		{
			fprintf(stderr,"Error:mode set failure.\n");
			goto set_failure;
		}
	}
	if(flags & nw_set_ping)
	{
		if(p_flag & set_timeout )
		{
			if(p_flag & set_interval)
			{
				if(ping.interval < ping.timeout)
				{
					ret = nw_ping_set(dev,&ping);
					if(ret)
					{
						fprintf(stderr,"Error: ping params set failure.\n");
						goto set_failure;
					}
				}else
				{
					fprintf(stderr,"Error:interval should be smaller than timeout.\n");
					return -1;	
				}
			}else{
					fprintf(stderr,"Error:both interval and timeout should be set.\n");
					return -1;
			}
		}else if( p_flag &set_interval)
		{
			fprintf(stderr,"Error:both interval and timeout should be set.\n");
			return -1;	
		}else 
		{
			//do nothing.
		}
	}
	if(flags & nw_set_self)
	{
		ret = nw_self_set(dev,&self);
		if(ret)
		{
			fprintf(stderr,"Error:ownid %s set failure.\n",self.peerid);
			goto set_failure;
		}
	}
	if(flags & nw_set_mptcp)
	{
		ret = nw_mptcp_set(dev,mptcp);
		if(ret)
		{
			fprintf(stderr,"Error:mptcp set failure.\n");
			goto set_failure;
		}
	}
	if(flags & nw_set_dhcp)
	{
		if(strcmp(dhcp.enable,"yes") == 0 && dhcp.startip && dhcp.endip)
		{
			ret = nw_type_read(dev,&mo);
			if(ret)
				goto set_failure;
			//printf("%d \n",mo.mode);
			if(mo.mode == NW_MODE_SERVER)
			{
				//printf("%s\n",mo.mode== NW_MODE_SERVER ? "server":"client");
				ret = nw_dhcp_set(dev,&dhcp);
				if(ret)
				{
					fprintf(stderr,"Error:dhcp set failure.\n");
					goto set_failure;
				}
			}else
			{
				fprintf(stderr,"Error:only server mode  can be configured.\n");
				return -1;
			}
		}else
		{
			fprintf(stderr,"All dhcp args (dhcp,ip,mask) should be set.\n");
			return -1;
		}
	}
	fprintf(stdout,"Success.\n");
	return 0;
param_ip_error:
	perror("Error:invalid ip address.\n");
	return -1;
param_mask_error:
	perror("Error:invalid mask address.\n");
	return -1;	
set_failure:
	printf("Fail.\n");
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
int on_off(const char *msg,const char *realval)
{
	fprintf(stderr,
		"Error: argument of \"%s\" must be \"on\" or \"off\", not \"%s\"\n",
		msg, realval);
	return -1;
}
int yes_no(const char *msg, const char *realval)
{
	fprintf(stderr,
		"Error: argument of \"%s\" must be \"yes\" or \"no\", not \"%s\"\n",
		msg, realval);
	return -1;
}
int cli_ser(const char *msg, const char *realval)
{
	fprintf(stderr,
		"Error: argument of \"%s\" must be \"client\" or \"server\", not \"%s\"\n",
		msg, realval);
	return -1;
}
int check_self(const char *id)
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
		fprintf(stdout,"compress     \t\t%s   \n",other->compress);
	if(is_other[1])
		fprintf(stdout,"budget 	\t\t%d    \n",other->budget);
	if(is_other[2])
		fprintf(stdout,"queuelen    \t\t%d    \n", other->queuelen);
	if(is_other[3])
		fprintf(stdout,"oneclient   \t\t%s    \n",strcmp(other->oneclient,"yes")==0?"yes":"no");
	if(is_other[4])
		fprintf(stdout,"log    	    \t\t%s    \n",strcmp(other->showlog,"yes")==0?"yes":"no");
	if(is_other[5])
		fprintf(stdout,"idletimeout \t\t%ds   \n",other->idletimeout);
	if(is_other[6])
		fprintf(stdout,"switchtime  \t\t%ds   \n",other->switchtime);
	if(is_other[7])
		fprintf(stdout,"autopeer  \t\t%s   \n",strcmp(other->autopeer,"yes")==0?"yes":"no");
	if(is_other[8])
		fprintf(stdout,"simpleroute \t\t%s\n",strcmp(other->simpleroute,"yes")==0?"yes":"no");
	if(is_other[9])
		fprintf(stdout,"isolate    \t\t%s\n",strcmp(other->isolate,"yes")==0?"yes":"no");
}
static void ping_print(struct nw_ping *ping,bool is_ping[])
{
	if(is_ping[0])
		fprintf(stdout,"interval   \t\t%dms   \n",ping->interval);
	if(is_ping[1])
		fprintf(stdout,"timeout    \t\t%dms   \n",ping->timeout);
}
static void peer_print(struct nw_peer_entry *entry,char *id,int idlen)
{
	char ipv4[16];
	int i;
	if(entry->head.type == NW_OPER_PEER )
	{
		for(i = 0 ; i < entry->count ; i++)
		{
			if(strcmp(entry->peerid[i],id) == 0)
			{
					inet_ntop(AF_INET,&entry->ip[i],ipv4,16);
					fprintf(stdout,"%-4d%-*s  %-15s   %-5u\n",i+1,idlen,entry->peerid[i],ipv4,entry->port[i]);
			}
		}
	}
}