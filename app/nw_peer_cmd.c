#include "utils.h"
#include "nw_cli.h"
#include "nw_err.h"

const char* peer_status_str[] =
{
	"Count",
	"PeerID",
	"address",
	"port",
	"active",
	"connecttime",
	"sendpackets",
	"sendbytes",
	"recvpackets",
	"recvbytes",
	"recvspeed",
	NULL,
};
static int nw_do_change(const char *,struct nw_peer_entry *);
static int nw_do_add(const char *, struct nw_peer_entry *);
static int nw_do_del(const char *, struct nw_peer_entry *);
static int nw_do_peer_list(const char *,struct nw_peer_entry *);
//[dev] nw1 peerid PEERID peerip PEERIP peerport PEERPORT
int nw_peer_change(int argc, char **argv)
{
	int ret;
	char *dev = NULL;
	char *peerid = NULL;
	char *peerip = NULL;
	struct nw_peer_entry *entry = (struct nw_peer_entry *)calloc(1,sizeof(struct nw_peer_entry));
	if(entry == NULL)
	{
		fprintf(stderr,"\ncalloc error.");
		return MEMERR;
	}
	while(argc > 0)
	{
		if(strcmp(*argv,"peerid") == 0)
		{
			NEXT_ARG();
			peerid = *argv;	
		}else if (strcmp(*argv,"peerip") == 0)
		{
			NEXT_ARG();
			peerip = *argv;
		}else if(strcmp(*argv,"peerport") == 0)
		{	
			NEXT_ARG();
			if(get_unsigned(&entry->port[0],*argv,0))
				invarg("Invalid \"port\" value\n",*argv);
		}else
		{
			if(strcmp(*argv,"dev") == 0)
				NEXT_ARG();
			if(dev)
				duparg2("dev",*argv);
			if(check_ifname(*argv))
				invarg("\"dev\" not a valid ifname", *argv);
			dev = *argv;
		}
		argc--;	argv++;
	}
	if(!dev)
	{
		fprintf(stderr,"Not enough information:\"dev\" argument is required.\n");
		exit(-1);
	}
	if(peerip||peerid)
	{
		if(peerip)
		{
			ret = inet_pton(AF_INET,peerip,&entry->ip[0]);
			if( ret < 0)
				return -1;
		}
		if(peerid)
			strcpy(entry->peerid[0],peerid);
		if(nw_do_change(dev,entry) < 0)
		{
			free(entry);
			return -1;	
		}
	}
	free(entry);
	return 0;
}

static int nw_do_change(const char *dev,struct nw_peer_entry *npe)
{
	int ret;
	strncpy(npe->head.devname,dev,IFNAMSIZ);
	npe->count = 1;
	npe->head.command = NW_COMM_PEER_CHANGE;
	npe->head.type = NW_OPER_PEER;
	ret = nw_ioctl((struct nw_oper_head *)npe);
	if(!ret)
		return -1;
	return 0;
}
  
//dev nw1 peerid p1,p2,p3
int nw_peer_del(int argc, char ** argv)
{
	struct nw_peer_entry *entry = calloc(1,sizeof(struct nw_peer_entry));
	int ret,i=0;
	char *cur = NULL;
	char *dev = NULL;
	while(argc > 0)
	{
		if(strcmp(*argv,"peerid") == 0)
		{
			NEXT_ARG();
			if(strlen(*argv) == 0)
				return PEERIDERR;
			else
			{
				for(cur = strtok(*argv,",");cur != NULL;cur=strtok(NULL,","),i++)
				{
					strcpy(entry->peerid[i],cur);
				}
				entry->count = i;
			}
		}else 
		{
			if(strcmp(*argv,"dev") == 0)
			{
				NEXT_ARG();
			}
			if(dev)
				duparg2("dev",*argv);
			if(check_ifname(*argv))
				invarg("\"dev\" not a valid ifname", *argv);
			dev = *argv;
		}
		argc--;	argv++;
	}
	if(!dev)
	{
		fprintf(stderr,"Not enough information:\"dev\" argument is required.\n");
		exit(-1);
	}
	if(entry->count)
	{

		if(nw_do_del(dev,(struct nw_oper_head*)entry) < 0)
		{
			free(entry);
			return PEERDELERR;
		}
	}
	free(entry);
	return 0;
}
static int nw_do_add(const char *dev,struct nw_peer_entry *npe)
{
	int ret;
	strncpy(npe->head.devname,dev,IFNAMSIZ);
	npe->count = 1;
	npe->head.command =  NW_COMM_PEER_ADD;
	npe->head.type = NW_OPER_PEER;
	ret = nw_ioctl((struct nw_oper_head*)npe);
	if(!ret)
		return -1;
	return 0;

}
static int nw_do_del(const char *dev,struct nw_peer_entry *npe)
{
	int ret;
	strncpy(npe->head.devname,dev,IFNAMSIZ);
	npe->head.command = NW_COMM_PEER_DEL;
	npe->head.type = NW_OPER_PEER;
	ret = nw_ioctl(npe);
	if(!ret)
		return -1;
	return 0;
}
//[dev] nw1 peerid PEERID peerip PEERIP  peerpot PORT
int nw_peer_add(int argc, char ** argv)
{
	struct nw_peer_entry entry;
	memset(&entry,0,sizeof(struct nw_peer_entry));
	int ret;
	char *dev =NULL;
	char *id = NULL;
	char *ip = NULL;
	u32 port;
	char *p,*q,**err = NULL;
	while(argc > 0) 
	{
		if(strcmp(*argv,"peerid") == 0 || strcmp(*argv,"id") == 0)
		{
			NEXT_ARG();
			if(id)
				duparg(id,*argv);
			id = *argv;
		}else if (strcmp(*argv,"peerip") == 0 || strcmp(*argv,"ip") == 0)
		{
			NEXT_ARG();
			if(ip)
				duparg(ip,*argv);
			ip = *argv;
		}else if(strcmp(*argv,"peerport") == 0 ||strcmp(*argv,"port") == 0)
		{	
			NEXT_ARG();
			if(port)
				duparg("port",*argv);
			if(get_unsigned(&port,*argv,0) || port > 65535)
				invarg("Invalid \"port\"\n",*argv);
		}else{
			if(strcmp(*argv,"dev") == 0)
			{
				NEXT_ARG();
			}
			if(dev)
				duparg2("dev",*argv);
			dev = *argv;
		}
		argc--;argv++;
	}
	if(!dev)
	{
		fprintf(stderr,"Not enough information:\"dev\" argument is required.\n");
		exit(-1);
	}
	if(ip&& id != NULL && port )
	{
		strcpy(entry.peerid[0],id);
		inet_pton(AF_INET,ip,&entry.ip[0]);
		entry.port[0] = port;
		if(nw_do_add(dev,&entry) < 0)
			return -1;
	}else
	{
		fprintf(stderr,"ip id port are all required .\n");
		exit(-1);
	}
	return 0;
}
//peerid p1,p2,p3
//type:NW_OPER_PEER command:NW_COMM_PEER_DEL
/*int nw_peer_list_del(int argc, char **argv)
{
	struct nw_peer_entry *peer = calloc(1,sizeof(struct nw_peer_entry));
	int ret,i;
	char token[MAX_PEER_NUMBER][MAX_PEERNAME_LENGTH];
	memset(token,0,sizeof(token));
	char *p,
		 *save_p =NULL,
		  **err = NULL;

	peer->count = i;
	peer.head.type =NW_OPER_PEER;
	peer.head.command=NW_COMM_PEER_DEL;
	ret = nw_peer_ioctl(&peer);
	if(ret)
	{
			printf("ioctl err.\n");
	}
	return ret;
}*/
// dev nw1 peerid 
//type:NW_OPER_PEER command:NW_COMM_PEER_LIST
int nw_peer_show(int argc, char **argv)
{
	int ret;
	char *peerid = NULL;
	char *dev = NULL;
	struct nw_peer_entry entry;
	memset(&entry,0,sizeof(struct nw_peer_entry));
	if(argc < 3)
		return -1;
	while(argc > 0)
	{
		if(strcmp(*argv,"peerid") == 0)
		{
			NEXT_ARG();
			peerid = *argv;
		}else 
		{
			if( strcmp(*argv,"dev") == 0)
				NEXT_ARG();
			if(dev)
				duparg2(*argv,"dev");
		}
		argc--;
		argv++;

	}

	return 0;
}