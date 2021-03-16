#include "nw_cli.h"
#include "utils.h"
#include "nw_err.h"
#include "manage.h"
int nw_ping_read(const char *dev ,struct nw_ping *ping)
{
	int ret;
	if(ping == NULL)
		return -1;
	if(dev == NULL)
		return DEVERR;
	strcpy(ping->head.devname,dev);
	ping->head.type = NW_OPER_PING;
	ping->head.command = NW_COMM_READ;
	ret = nw_ioctl((struct nw_oper_head*)ping);
	return ret;
}
int nw_other_read(const char *dev,struct nw_other* other)
{	
	int ret ;
	strncpy(other->head.devname,dev,IFNAMSIZ);
	other->head.type = NW_OPER_OTHER;
	other->head.command = NW_COMM_READ;
	ret = nw_ioctl(&other->head);
	return ret;
}
int nw_bind_read(const char *dev,struct nw_bind* bind)
{	
	int ret ;
	strncpy(bind->head.devname,dev,IFNAMSIZ);
	bind->head.type = NW_OPER_BIND;
	bind->head.command = NW_COMM_READ;
	ret = nw_ioctl(&bind->head);
	return ret;
}
int nw_type_read(const char *dev, struct nw_type* type)
{
	int ret;
	strncpy(type->head.devname,dev,IFNAMSIZ);
	type->head.type = NW_OPER_TYPE;
	type->head.command = NW_COMM_READ;
	ret = nw_ioctl(&type->head);
	return ret;
}
int nw_other_set(const char *dev,struct nw_other* other)
{
	int ret ;
	strncpy(other->head.devname,dev,IFNAMSIZ);
	other->head.type = NW_OPER_OTHER;
	other->head.command = NW_COMM_SET;
	ret = nw_ioctl(&other->head);
	return ret;
}
int nw_bind_set(const char *dev, struct nw_bind* bind)
{	
	int ret ;
	strcpy(bind->head.devname,dev);
	bind->head.type = NW_OPER_BIND;
	bind->head.command = NW_COMM_SET;
	ret = nw_ioctl(&bind->head);
	return ret;
}
int nw_type_set(const char *dev, struct nw_type* mo)
{
	int ret; 
	strcpy(mo->head.devname,dev);
	mo->head.type = NW_OPER_TYPE;
	mo->head.command = NW_COMM_SET;
	ret = nw_ioctl(&mo->head);
	return ret;
}
int nw_ping_set(const char *dev, struct nw_ping *ping)
{
	int ret;
	strcpy(ping->head.devname,dev);
	ping->head.type = NW_OPER_PING;
	ping->head.command = NW_COMM_SET;
	ret = nw_ioctl(&ping->head);
	return ret;
}
int nw_ioctl(struct nw_oper_head *head)
{
    struct ifreq req;
    int sock,ret;
    
	memset(&req,0,sizeof(req));
    req.ifr_data = (void *)head;
    strcpy(req.ifr_name,head->devname);
	
	sock = socket(AF_INET,SOCK_DGRAM,0);
    if(!sock)
    {
        return -1;
    }
    ret = ioctl(sock,NW_OPER,&req);
    if(ret) 
	{
			printf("ioctl err=%d.\n",errno);
			close(sock);
			return -1;
	}
	close(sock);
	return !head->result;
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
				return -1;
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
void do_read(struct nw_oper_head *head)
{	
	if(head->type == NW_OPER_PEER)
	{
		struct nw_peer_entry *entry = (struct nw_peer_entry *) head;
		char ipv4[16];
		int i;
		fprintf(stdout,"No.   id\tpeerip\t\tpeerport\n");
		for(i = 0 ; i < entry->count ; i++)
		{
			if(inet_ntop(AF_INET,&entry->ip[i],ipv4,16) < 0)
				return -1;
			fprintf(stdout,"%-5d %-10s%-16s%-10u\n",i,entry->peerid[i],ipv4,entry->port[i]);
		}
	}else if(head->type == NW_OPER_BIND)
	{
		struct nw_bind *bind = (struct nw_bind *) head;
		fprintf(stdout,"bindport   \t%-10d   \n",bind->port);
	}else if (head->type == NW_OPER_PING)
	{
		struct nw_ping *ping = (struct nw_ping *)head;
		fprintf(stdout,"interval   \t%-10d   \n",ping->interval);
		fprintf(stdout,"timeout    \t%-10d   \n",ping->timeout);
	}else if(head->type == NW_OPER_TYPE)
	{
		struct nw_type *type = (struct nw_type *)head;
		fprintf(stdout,"mode       \t%-10s   \n",mode_str(type->mode));
	}else if(head->type == NW_OPER_DEVSTAT)
	{
		char *dev = NULL;
		int ret ;
		struct nw_dev_stat *stat = (struct nw_dev_stat*)head;
		ret = nw_search_if(dev);
		if(ret)
			fprintf(stderr,"dev not found.");
		nw_stat_dev(dev);
	}
	else if(head->type == NW_OPER_OTHER )
	{
		struct nw_other *other = (struct nw_other *)head;
		fprintf(stdout,"bufflen    \t%-10d    \n",other->bufflen);
		fprintf(stdout,"maxbufflen \t%-10d    \n",other->maxbufflen);
		fprintf(stdout,"oneclient  \t%-10s    \n",strcmp(other->oneclient,"yes")== 0?"yes":"no");
		fprintf(stdout,"log   	   \t%-10s    \n",strcmp(other->showlog,"yes") ==0?"yes":"no");
		fprintf(stdout,"queuelen   \t%-10d    \n",other->queuelen);
		fprintf(stdout,"idletimeout\t%-10d    \n",other->idletimeout);
		fprintf(stdout,"batch      \t%-10d    \n",other->batch?other->batch:0);
	}
	return ;
}

int nw_self_ownid(const char *dev, char *ownid)
{
	return 0;
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