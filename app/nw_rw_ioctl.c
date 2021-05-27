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
	assert(dev != NULL);
	assert(entry != NULL);
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
	assert(dev != NULL);
	assert(entry != NULL);
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
	assert(dev != NULL);
	assert(npe != NULL);
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
	assert(dev != NULL);
	assert(npe != NULL);
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
	assert(dev != NULL);
	assert(npe != NULL);
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
	assert(dev != NULL);
	assert(ping != NULL);
	strcpy(ping->head.devname,dev);
	ping->head.type = NW_OPER_PING;
	ping->head.command = NW_COMM_READ;
	ret = nw_ioctl((struct nw_oper_head*)ping);
	return ret;
}
int nw_other_read(const char *dev,struct nw_other* other)
{	
	int ret ;
	assert(dev != NULL);
	assert(other != NULL);
	strncpy(other->head.devname,dev,IFNAMSIZ);
	other->head.type = NW_OPER_OTHER;
	other->head.command = NW_COMM_READ;
	ret = nw_ioctl(&other->head);
	return ret;
}
int nw_bind_read(const char *dev,struct nw_bind* bind)
{	
	int ret ;
	assert(dev != NULL);
	assert(bind != NULL);
	strncpy(bind->head.devname,dev,IFNAMSIZ);
	bind->head.type = NW_OPER_BIND;
	bind->head.command = NW_COMM_READ;
	ret = nw_ioctl(&bind->head);
	return ret;
}
int nw_type_read(const char *dev, struct nw_type* type)
{
	int ret;
	assert(dev != NULL);
	assert(type != NULL);
	strncpy(type->head.devname,dev,IFNAMSIZ);
	type->head.type = NW_OPER_TYPE;
	type->head.command = NW_COMM_READ;
	ret = nw_ioctl(&type->head);
	return ret;
}
int nw_self_read(const char *dev, struct nw_self*self)
{
	int ret;
	assert(dev != NULL);
	assert(self != NULL);
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
	assert(dev != NULL);
	assert(other != NULL);
	strncpy(other->head.devname,dev,IFNAMSIZ);
	other->head.type = NW_OPER_OTHER;
	other->head.command = NW_COMM_SET;
	ret = nw_ioctl(&other->head);
	return ret;
}
int nw_bind_set(const char *dev, struct nw_bind* bind)
{	
	int ret ;
	assert(dev != NULL);
	assert(bind != NULL);
	strcpy(bind->head.devname,dev);
	bind->head.type = NW_OPER_BIND;
	bind->head.command = NW_COMM_SET;
	ret = nw_ioctl(&bind->head);
	return ret;
}
int nw_type_set(const char *dev, struct nw_type* mo)
{
	int ret; 
	assert(dev != NULL);
	assert(mo != NULL);
	strcpy(mo->head.devname,dev);
	mo->head.type = NW_OPER_TYPE;
	mo->head.command = NW_COMM_SET;
	ret = nw_ioctl(&mo->head);
	return ret;
}
int nw_ping_set(const char *dev, struct nw_ping *ping)
{
	int ret;
	assert(dev != NULL);
	assert(ping != NULL);
	strcpy(ping->head.devname,dev);
	ping->head.type = NW_OPER_PING;
	ping->head.command = NW_COMM_SET;
	ret = nw_ioctl(&ping->head);
	return ret;
}
int nw_self_set(const char *dev, struct nw_self*self)
{
	int ret;
	assert(dev != NULL);
	assert(self != NULL);
	strncpy(self->head.devname,dev,IFNAMSIZ);
	self->head.type = NW_OPER_SELF;
	self->head.command = NW_COMM_SET;
	ret = nw_ioctl(&self->head);
	return ret;
}
int nw_dhcp_set(const char *dev,struct nw_dhcp *dhcp)
{
	int ret; 
	assert(dev != NULL);
	assert(dhcp != NULL);
	strncpy(dhcp->head.devname,dev,IFNAMSIZ);
	dhcp->head.type = NW_OPER_DHCP;
	dhcp->head.command = NW_COMM_SET;
	ret = nw_ioctl(&dhcp->head);
	return ret;
}
int nw_dhcp_read(const char *dev,struct nw_dhcp *dhcp)
{
	int ret; 
	assert(dev != NULL);
	assert(dhcp != NULL);
	strncpy(dhcp->head.devname,dev,IFNAMSIZ);
	dhcp->head.type = NW_OPER_DHCP;
	dhcp->head.command = NW_COMM_READ;
	ret = nw_ioctl(&dhcp->head);
	return ret;
}

int nw_mptcp_read(char *dev)
{
	char mptcp_cmd[50];
	sprintf(mptcp_cmd,"ip link show %s|grep -o NOMULTIPATH ",dev);
	if(system(mptcp_cmd))
	{
		return -1;
	}
	return 0;
}
int nw_mptcp_set(char *dev ,bool on_off)
{
	char mptcp_cmd[50];
	sprintf(mptcp_cmd,"ip link set %s multipath %s",dev,on_off?"on":"off");
	if(system(mptcp_cmd))
	{
		fprintf(stderr,"multipath exec err.");
		return -1;
	}
	return 0;
}
int nw_ioctl(struct nw_oper_head *head)
{
    struct ifreq req;
    int sock,ret  = 0;
    
	memset(&req,0,sizeof(req));
    req.ifr_data = (void *)head;
    strcpy(req.ifr_name,head->devname);
	sock = socket(AF_INET,SOCK_DGRAM,0);
    if(!sock)
    {
		fprintf(stderr,"[%s] param error",__FUNCTION__);
        return -1;
    }
    ret = ioctl(sock,NW_OPER,&req);
	if(ret)
	{
		fprintf(stderr,"%s type:%d command:%d %s\n",req.ifr_name,head->type,head->command,strerror(errno));
	}
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
	struct nw_peer_entry *entry = (struct nw_peer_entry *) head;
	char ipv4[16];
	char start[16];
	char end[16];
	char mask[16];
	//int max=0;
	int i;
	int maxid = 0;
	if(head->type == NW_OPER_PEER)
	{
		for(i = 0 ; i < entry->count ; i++)
		{
			maxid = maxid > strlen(entry->peerid[i]) ? maxid : strlen(entry->peerid[i]);
		}
		fprintf(stdout,"No. Id%-*sIP%-*sPort%-*s \n",maxid," ",15," ",5," ");
		for(i = 0 ; i < entry->count ; i++)
		{
			//max = max > strlen(entry->peerid[i]) ? max : strlen(entry->peerid[i]);
			inet_ntop(AF_INET,&entry->ip[i],ipv4,16);
			fprintf(stdout,"%-4d%-*s  %-15s  %-5u\n",i+1,maxid,entry->peerid[i],ipv4,entry->port[i]);
			//printf("max %d\n",max);
		}
	}else if(head->type == NW_OPER_TYPE)
	{
		struct nw_type *type = (struct nw_type *)head;
		fprintf(stdout,"mode       	\t%s	\n",mode_str(type->mode));
	}else if(head->type == NW_OPER_BIND)
	{
		struct nw_bind *bind = (struct nw_bind *) head;
		fprintf(stdout,"bindport	\t%d   	\n",bind->port);
	}else if (head->type == NW_OPER_PING)
	{
		struct nw_ping *ping = (struct nw_ping *)head;
		fprintf(stdout,"interval	\t%dms	\n",ping->interval);
		fprintf(stdout,"timeout		\t%dms	\n",ping->timeout);
	}else if(head->type == NW_OPER_DEVSTAT)
	{
		char *dev = NULL;
		int ret ;
		ret = nw_search_if(dev);
		if(ret)
			fprintf(stderr,"dev not found.\n");
	}
	else if(head->type == NW_OPER_OTHER )
	{
		struct nw_other *other = (struct nw_other *)head;
		fprintf(stdout,"budget		\t%d	\n",other->budget);
		fprintf(stdout,"oneclient	\t%s	\n",strcmp(other->oneclient,"yes")== 0?"yes":"no");
		fprintf(stdout,"log		\t%s    \n",strcmp(other->showlog,"yes") ==0?"yes":"no");
		fprintf(stdout,"compress	\t%s	\n",strcmp(other->compress,"yes") ==0?"yes":"no");
		fprintf(stdout,"simpleroute	\t%s	\n",strcmp(other->simpleroute,"yes") ==0?"yes":"no");
		fprintf(stdout,"autopeer	\t%s 	\n",strcmp(other->autopeer,"yes") == 0 ?"yes":"no");
		fprintf(stdout,"isolate		\t%s 	\n",strcmp(other->isolate,"yes") == 0 ?"yes":"no");
		fprintf(stdout,"queuelen	\t%d    \n",other->queuelen);
		fprintf(stdout,"idletimeout	\t%ds   \n",other->idletimeout);
		fprintf(stdout,"switchtime 	\t%ds	\n",other->switchtime);
	}else if(head->type == NW_OPER_SELF)
	{
		struct nw_self *self = (struct nw_self *) head;
		fprintf(stdout,"ownid	   	\t%s	  \n",strlen(self->peerid)?self->peerid:"None");
	}else if(head->type == NW_OPER_DHCP)
	{
		struct nw_dhcp *dhcp =(struct nw_dhcp *)head;
		inet_ntop(AF_INET,&dhcp->startip,start,16);
		inet_ntop(AF_INET,&dhcp->endip,end,16);
		inet_ntop(AF_INET,&dhcp->mask,mask,16);
		fprintf(stdout,"dhcp		\t%s		\n",strcmp(dhcp->enable,"yes")== 0 ?"yes":"no");
		fprintf(stdout,"dhcp-startip	\t%-16s		\n",start);
		fprintf(stdout,"dhcp-endip	\t%-16s		\n",end);
		fprintf(stdout,"dhcp-mask	\t%-16s		\n",mask);
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