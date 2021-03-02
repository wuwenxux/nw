#include "nw_cli.h"
#include "utils.h"
#include <err.h>

static int get_nw_mode(const char *mode);
static int yes_no(const char *msg, const char *realval);
static int cli_ser(const char *msg, const char *realval);
int nw_ioctl(struct nw_oper_head *p);
static int get_nw_mode(const char *mode)
{
	if (strcasecmp(mode, "client") == 0 )
		return NW_MODE_CLIENT;
	else if (strcasecmp(mode, "server") == 0)
		return NW_MODE_SERVER;
	else 
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
int nw_other_set(const char *dev,struct nw_other* other)
{
	int ret ;
	strncpy(other->head.devname,dev,IFNAMSIZ);
	other->head.type = NW_OPER_OTHER;
	other->head.command = NW_COMM_SET;
	ret = nw_ioctl((struct nw_head *)other);
	if(!ret)
		return -1;
	return 0;
}
int nw_bind_set(const char *dev, struct nw_bind* bind)
{	
	int ret ;
	strcpy(bind->head.devname,dev);
	bind->head.type = NW_OPER_BIND;
	bind->head.command = NW_COMM_SET;
	ret = nw_ioctl((struct nw_oper_head *)bind);
	if(!ret)
		return -1;
	return 0;
}
int nw_type_set(const char *dev, struct nw_type* mo)
{
	int ret; 
	strcpy(mo->head.devname,dev);
	mo->head.type = NW_OPER_TYPE;
	mo->head.command = NW_COMM_SET;
	ret = nw_ioctl((struct nw_oper_head*)mo);
	if(!ret)
		return -1;
	return 0;
}
int nw_ping_set(const char *dev, struct nw_ping *ping)
{
	int ret;
	strcpy(ping->head.devname,dev);
	ping->head.type = NW_OPER_PING;
	ping->head.command = NW_COMM_SET;
	ret = nw_ioctl((struct nw_oper_head*)ping);
	if(!ret)
		return -1;
	return 0;
}
int nw_search_if( char *dev)
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
	while(fgets(buf,512,fp) != NULL)
	{
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
				strcpy(dev,&n[1]);
				return 0;
			}
		}
	}
	pclose(fp);
	
	/* not exist net device */
	return -1;
}
int nw_ioctl(struct nw_oper_head *head)
{
    struct ifreq req;
    int sock,ret;
	do_read(head);
    
  	/* 
	memset(&req,0,sizeof(req));
    req.ifr_data = (void *)head;
    strcpy(req.ifr_name,head->devname);
	
	sock = socket(AF_INET,SOCK_DGRAM,0);
    if(!sock)
    {
		fprintf(stderr,"peer sockert err.\n");
        return SOCKERR;
    }
	*/

    /*ret = ioctl(sock,NW_OPER,&req);
    if(ret) 
	{
		fprintf(stderr,"ioctl err");
		return ret ;
	}
	if(head->result)
	{
		fprintf(stdout,"ioctl success %s oper %d success\n",head->devname,head->command);
	}else
	{
		fprintf(stderr,"nw %s oper %d error\n",head->devname,head->command);
		return -1;
	}
    close(sock);
	*/
    return 0;
}

int do_read(struct nw_oper_head *head)
{
//	struct nw_oper_head *head;
	fprintf(stdout,"\n           NW Args (%s)   \n",head->devname);
	fprintf(stdout,"--------------------------------\n");
	if(head->type == NW_OPER_PEER)
	{
		struct nw_peer_entry *entry = (struct nw_peer_entry *) head;
		char ipv4[20];
		int i;
		for(i = 0 ; i < entry->count ; i++)
		{
			if(inet_ntop(AF_INET,&entry->ip[i],ipv4,16) < 0)
				return -1;
			fprintf(stdout," peerid[%d]\t%-16s  \n",i,entry->peerid[i]);
		//	fprintf(stdout,"-----------------------------------------\n");
			fprintf(stdout," peerip    \t%-16s  \n",ipv4);
		//	fprintf(stdout,"-----------------------------------------\n");
			fprintf(stdout," peerport  \t%-16d  \n",entry->port[i]);
			fprintf(stdout,"--------------------------------\n");
		}
	}else if(head->type == NW_OPER_SELF)
	{
			struct nw_self  *self = (struct nw_self *)head;
			fprintf(stdout," ownid    \t%-10s\n",self->peerid);
			fprintf(stdout,"--------------------------------\n");
	}
	else if(head->type == NW_OPER_BIND)
	{
			struct nw_bind *bind = (struct nw_bind *) head;
			fprintf(stdout," bindport \t%-10d\n",bind->port);
			fprintf(stdout,"-----------------------------------------\n");
	}else if (head->type == NW_OPER_PING)
	{
			struct nw_ping *ping = (struct nw_ping *)head;
			fprintf(stdout," interval \t%-10d\n",ping->interval);
			fprintf(stdout,"-----------------------------------------\n");
			fprintf(stdout," timeout  \t%-10d\n",ping->timeout);
	}else if(head->type == NW_OPER_TYPE)
	{
			struct nw_type *type = (struct nw_type *)head;
			fprintf(stdout," mode     \t%-10s\n",mode_str(type->mode));
			fprintf(stdout,"-----------------------------------------\n");
	}else if(head->type == NW_OPER_DEVSTAT )
	{
		char *dev = NULL;
		int ret ;
		struct nw_dev_stat *stat = (struct nw_dev_stat*)head;
		ret = nw_search_if(dev);
		if(ret)
			fprintf(stderr,"dev not found.");
		nw_dev_show_statistic(dev);
	}
	else if(head->type == NW_OPER_OTH  ER )
	{
		struct nw_other *other = (struct nw_other *)head;
		if(other->bufflen)
		{
			fprintf(stdout," bufflen    \t%d\n",other->bufflen);
		}
		if(other->maxbufflen)
		{
			fprintf(stdout," maxbufflen \t%d\n",other->maxbufflen);
		}
		if(strcmp(other->oneclient,"yes") == 0 || strcmp(other->oneclient,"no") == 0)
		{
			fprintf(stdout," oneclient  \t%s\n",other->oneclient);
		}	
		if(strcmp(other->showlog,"yes") == 0 || strcmp(other->showlog,"no") == 0)
		{
			fprintf(stdout," showlog    \t%s\n",other->showlog);
		}
		if(other->queuelen)
		{
			fprintf(stdout," queuelen   \t%d\n",other->queuelen);
		}
		if(other->idletimeout)
		{
			fprintf(stdout," idletimeout\t%d\n",other->idletimeout);
		}	
		if(other->batch)
		{
			fprintf(stdout," batch      \t%d\n",other->batch);
		}
			fprintf(stdout,"--------------------------------\n");	
	}
}

int nw_dev_show(int argc, char **argv)
{
    int ret;
    
    return 0;
}
int nw_self_ownid(const char *dev, char *ownid)
{
	struct nw_self own;
	memset(&own,0,sizeof(own));
	own.head.type = NW_OPER_SELF;
	own.head.command  = NW_COMM_SET;
	strcpy(own.peerid,ownid);
	strcpy(own.head.devname,dev);
	if(nw_ioctl((struct nw_oper_head *)&own) < 0)
	{
		return -1;	
	}
	return 0;
}
int nw_dev_set(int argc, char **argv)
{
	struct nw_other other;
	struct nw_bind bind;
	struct nw_type mo;
	struct nw_ping ping;
	bool do_ping = false;
	char *dev = NULL;
	char *ownid = NULL;
	int ret ;

	memset(&other,0,sizeof(struct nw_other));
	memset(&bind,0,sizeof(struct nw_bind));
	memset(&mo,0,sizeof(struct nw_type));
	memset(&ping,0,sizeof(struct nw_ping));

	while(argc > 0)
	{
		if(strcmp(*argv,"bufflen") == 0)
		{
			NEXT_ARG();
			if( other.bufflen != 0)
				duparg("bufflen",*argv);
			if(get_unsigned(&other.bufflen,*argv,0) || other.bufflen > 102400 || other.bufflen < 64)
				invarg("Invalid \"bufflen\" value\n",*argv);	
		}else if(matches(*argv,"maxbufflen") == 0 || matches(*argv,"maxblen") == 0)
		{
			NEXT_ARG();
			if(get_unsigned(&other.maxbufflen,*argv,0) )
				invarg("Invalid \"maxbufflen\" value\n",*argv);
		}else if(matches(*argv,"queuelen") == 0|| matches(*argv,"qlen") == 0)
		{
			NEXT_ARG();
			if(get_unsigned(&other.queuelen,*argv,0) || other.queuelen > 1000000 || other.queuelen < 1000)
				invarg("Invalid \"maxbufflen\" value\n",*argv);
		}else if(matches(*argv,"oneclient") == 0|| matches(*argv,"onecli") == 0)
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
		}else if (matches(*argv,"log") == 0)
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
		}else if (matches(*argv,"batch") == 0 || matches(*argv,"bat") == 0)
		{
			NEXT_ARG();
			if(get_unsigned(&other.batch,*argv,0) || other.batch >200 ||other.batch < 10)
				invarg("Invalid \"batch\" value\n",*argv);
		}else if (matches(*argv,"idletimeout") == 0 || matches(*argv,"idle") == 0) //ping
		{
			NEXT_ARG();
			if(get_unsigned(&other.idletimeout,*argv,0) || other.idletimeout < 30 )
				invarg("Invalid \"idletimeout\" value\n",*argv);
		}else if( matches(*argv,"mode") == 0) //type
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
		}else if( matches(*argv,"bindport") == 0)   //bind
		{
			NEXT_ARG();
			if(get_unsigned(&bind.port,*argv,0))
			{
				invarg("Invalid \"bindport \" value\n",*argv);
			}
		}
		else if (matches(*argv,"interval") == 0)
		{
			NEXT_ARG();
			if(get_unsigned(&ping.interval,*argv,0))
			{
				invarg("Invalid \"interval \" value\n",*argv);
			}
		}
		else if(matches(*argv,"timeout") == 0)
		{
			NEXT_ARG();
			if(get_unsigned(&ping.timeout,*argv,0) )
				invarg("Invalid \"timeout \" value\n",*argv);
			do_ping = true;
		}else if(matches(*argv,"ownid") == 0)
		{
			NEXT_ARG();
			if(ownid)
				duparg("ownid",*argv);
			ownid = *argv;
		}
		else{
			if(strcmp(*argv,"dev") == 0 )
				NEXT_ARG();
			else if (strcmp(*argv,"help") == 0)
				nw_usage();
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
	if(other.batch || other.idletimeout ||other.bufflen||other.maxbufflen || other.queuelen|| strlen(other.showlog)!= 0||strlen(other.oneclient) != 0)
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
	if(do_ping)
	{
		if(nw_ping_set(dev,&ping) < 0)
			return -1;
	}
	if(strlen(ownid) != 0)
	{
		if(nw_self_ownid(dev,ownid) < 0)
			return -1; 
	}
	return 0 ;
}
int nw_dev_del(int argc, char **argv)
{
    return 0;
}

int nw_dev_close(int argc, char **argv)
{
	char *dev = NULL;
	if(argc > 2)
	{
		fprintf(stderr,"Invalid argument format.\n");
		exit(-1);
	}
	if(strcmp(*argv,"dev") == 0)
	{
		NEXT_ARG();
	}
	if(dev)
		 	duparg2("dev",*argv);
	if(check_ifname(*argv))
			invarg("\"dev\" not a valid ifname",*argv);
	dev = *argv;

    return 0;
}