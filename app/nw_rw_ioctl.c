#include "nw_cli.h"
#include "utils.h"
#include "nw_err.h"
#include "manage.h"
#include <err.h>
/*peer ioctl*/
//dev connect peerid
int nw_do_connect(const char *dev, struct nw_peer_entry *entry)
{
	int ret;
	strcpy(entry->head.devname,dev);
	entry->head.command = NW_COMM_SET;
	entry->head.type = NW_COMM_PEER_CONNECT;
	ret = nw_ioctl(&entry->head);
	return ret;
}
//show dev peer
int nw_do_peer_list(const char *dev ,struct nw_peer_entry *entry)
{
	int ret;
	strcpy(entry->head.devname,dev);
	entry->head.command = NW_COMM_PEER_LIST;
	entry->head.type = NW_OPER_PEER;
	ret = nw_ioctl(&entry->head);
	return ret;
}
//add dev peerid
int nw_do_add(const char *dev,struct nw_peer_entry *npe)
{
	int ret;
	strncpy(npe->head.devname,dev,IFNAMSIZ);
	npe->head.command =  NW_COMM_PEER_ADD;
	npe->head.type = NW_OPER_PEER;
	ret = nw_ioctl((struct nw_oper_head*)npe);
	return ret;
}
//change dev peer
int nw_do_change(const char *dev,struct nw_peer_entry *npe)
{
	int ret;
	strncpy(npe->head.devname,dev,IFNAMSIZ);
	npe->count = 1;
	npe->head.command = NW_COMM_PEER_CHANGE;
	npe->head.type = NW_OPER_PEER;
	ret = nw_ioctl((struct nw_oper_head *)npe);
	return ret;
}
//del dev peerid 
int nw_do_del(const char *dev,struct nw_peer_entry *npe)
{
	int ret;
	strncpy(npe->head.devname,dev,IFNAMSIZ);
	npe->head.command = NW_COMM_PEER_DEL;
	npe->head.type = NW_OPER_PEER;
	ret = nw_ioctl((struct nw_oper_head *)npe);
	return ret;
}
/*ping,bind,other,type,self read ioctl*/
int nw_ping_read(const char *dev ,struct nw_ping *ping)
{
	int ret;
	if(ping == NULL)
		return -1;
	if(dev == NULL)
		return DEV_NOT_FOUND;
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
int nw_self_read(const char *dev, struct nw_self*self)
{
	int ret;
	strncpy(self->head.devname,dev,IFNAMSIZ);
	self->head.type = NW_OPER_SELF;
	self->head.command = NW_COMM_READ;
	ret = nw_ioctl(&self->head);
	return ret;
}
/*other,ping,bind,type,sef set ioctl*/
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
int nw_self_set(const char *dev, struct nw_self*self)
{
	int ret;
	strncpy(self->head.devname,dev,IFNAMSIZ);
	self->head.type = NW_OPER_SELF;
	self->head.command = NW_COMM_SET;
	ret = nw_ioctl(&self->head);
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
		fprintf(stderr,"dev:%s,result :%d \n",head->devname,head->result);
		goto Failed;
	}
	goto Success;
Failed:
	close(sock);
	return -1;
Success:
	close(sock);
	return !head->result;
}
int nw_search_if( char *dev)
{
	FILE *fp;
	char buf[512];
	char *cmdline = "ip link";
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
	
	/* net device is not exist  */
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
			inet_ntop(AF_INET,&entry->ip[i],ipv4,16);
			fprintf(stdout,"%-5d %-10s%-16s%-10u\n",i+1,entry->peerid[i],ipv4,entry->port[i]);
		}
	}else if(head->type == NW_OPER_BIND)
	{
		struct nw_bind *bind = (struct nw_bind *) head;
		fprintf(stdout,"bindport   \t%d   \n",bind->port);
	}else if (head->type == NW_OPER_PING)
	{
		struct nw_ping *ping = (struct nw_ping *)head;
		fprintf(stdout,"interval   \t%ds   \n",ping->interval);
		fprintf(stdout,"timeout    \t%dms   \n",ping->timeout);
	}else if(head->type == NW_OPER_TYPE)
	{
		struct nw_type *type = (struct nw_type *)head;
		fprintf(stdout,"mode       \t%s   \n",mode_str(type->mode));
	}else if(head->type == NW_OPER_DEVSTAT)
	{
		char *dev = NULL;
		int ret ;
//		struct nw_dev_stat *stat = (struct nw_dev_stat*)head;
		ret = nw_search_if(dev);
		if(ret)
			fprintf(stderr,"dev not found.");
//		nw_stat_dev(dev);
	}
	else if(head->type == NW_OPER_OTHER )
	{
		struct nw_other *other = (struct nw_other *)head;
		fprintf(stdout,"bufflen    \t%dK    \n",other->bufflen);
		fprintf(stdout,"budget 	   \t%d    \n",other->budget);
		fprintf(stdout,"oneclient  \t%s    \n",strcmp(other->oneclient,"yes")== 0?"yes":"no");
		fprintf(stdout,"log   	   \t%s    \n",strcmp(other->showlog,"yes") ==0?"yes":"no");
		fprintf(stdout,"queuelen   \t%dK    \n",other->queuelen);
		fprintf(stdout,"idletimeout\t%ds    \n",other->idletimeout);
		fprintf(stdout,"batch      \t%d   \n",other->batch?other->batch:0);
		fprintf(stdout,"switchtime \t%ds	  \n",other->switchtime);
	}else if(head->type == NW_OPER_SELF)
	{
		struct nw_self *self = (struct nw_self *) head;
		fprintf(stdout,"ownid	   \t%-10s	  \n",self->peerid);
	}
	return ;
}

int nw_dev_del(int argc, char **argv)
{
    return 0;
}
int nw_dev_close(int argc, char **argv)
{
	/*
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
    */
	return 0;
}