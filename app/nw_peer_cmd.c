#include "utils.h"
#include "nw_cli.h"
#include "nw_err.h"
#include "manage.h"
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

//[dev] nw1 peerid PEERID peerip PEERIP peerport PEERPORT
int nw_peer_change(int argc, char **argv)
{
	char *dev = NULL;
	char peerid[MAX_PEERNAME_LENGTH];
	char peerip[IPV4NAMESIZE];
	u16   peerport;
	struct nw_peer_entry *entry = (struct nw_peer_entry *)calloc(1,sizeof(struct nw_peer_entry));
	if(entry == NULL)
	{
		fprintf(stderr,"\ncalloc error.");
		return MEMERR;
	}
	if(argc > 8 || argc < 7)
	{
		fprintf(stderr,"Num of argc is not right.\n");
		goto FAILED;
	}
	while(argc > 0)
	{
		if(strcmp(*argv,"peerid") == 0)
		{
			NEXT_ARG();
			strcpy(peerid,*argv);
		}else if (strcmp(*argv,"peerip") == 0 || strcmp(*argv,"ip") == 0)
		{
			NEXT_ARG();
			if(check_ipv4(*argv))
			{
				free(entry);
				invarg("invalid ip addr \n",*argv);
			}
			strcpy(peerip,*argv);

		}else if(strcmp(*argv,"peerport") == 0 || strcmp(*argv,"port") == 0)
		{	
			NEXT_ARG();
			if(get_unsigned16(&peerport,*argv,0))
			{
				free(entry);
				invarg("Invalid \"port\" value\n",*argv);
			}
		}else
		{
			if(strcmp(*argv,"dev") == 0)
				NEXT_ARG();
			if(dev)
			{
				free(entry);
				duparg2("dev",*argv);
			}
			if(check_ifname(*argv) )
			{
				free(entry);
				invarg("\"dev\" not a valid ifname", *argv);
			}
			if(check_nw_if(*argv))
			{
				free(entry);
				invarg("not a running nw dev.",*argv);
			}
			dev = *argv;
		}
		argv++;
		argc--;	
	}
	if(!dev)
	{
		fprintf(stderr,"Not enough information:\"dev\" argument is required.\n");
		goto FAILED;
	}
	if(strlen(peerid) >0 && is_exist(dev,peerid))
	{
		strcpy(entry->peerid[0],peerid);
		inet_pton(AF_INET,peerip,&entry->ip[0]);
		entry->port[0] = peerport;
		if(nw_do_change(dev,entry) < 0)
		{
			goto FAILED;
		}
		printf("Success!\n");
	}else
	{
		fprintf(stderr,"Error: peerid \'%s\' of Device \'%s\' is not exist.\n",peerid,dev);
	}
	goto SUCCESS;
SUCCESS:
	free(entry);
	return 0;
FAILED:
	free(entry);
	return -1;
}

//dev nw1 peerid p1,p2,p3
int nw_peer_del(int argc, char ** argv)
{
	struct nw_peer_entry *entry = calloc(1,sizeof(struct nw_peer_entry));
	int i=0;
	char *cur = NULL;
	char *dev = NULL;
	if(argc < 3 || argc > 4)
	{
		fprintf(stderr,"number of argc is not valid.\n");
		return -1;
	}
	while(argc > 0)
	{
		if(strcmp(*argv,"peerid") == 0)
		{
			NEXT_ARG();
			if(strlen(*argv) == 0)
			{
				free(entry);
				return PEERIDERR;
			}
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
			{
				free(entry);
				duparg2("dev",*argv);
			}
			if(check_ifname(*argv) )
			{
				free(entry);
				invarg("\"dev\" not a valid ifname", *argv);
			}
			if(check_nw_if(*argv))
			{
				free(entry);
				invarg("not a running nw dev.",*argv);
			}
			dev = *argv;
		}
		argc--;	argv++;
	}
	if(!dev)
	{
		fprintf(stderr,"Not enough information:\"dev\" argument is required.\n");
		goto FAILED;
	}
	if(entry->count)
	{

		if(nw_do_del(dev,entry) < 0)
		{
			goto FAILED;
		}
		else 
			printf("Success!\n");
	}
		goto SUCCESS;
SUCCESS:
	free(entry);
	return 0;
FAILED:
	free(entry);
	return -1;
}
//validate peer str: p1,ip,port
int check_opt_peer(const char *dev, char *value,char *peerid,u32 *nl_ip, u16 *v_port)
{
	assert(dev != NULL);
	assert(value != NULL);
	char *ip = NULL;
	char *id = NULL;
	char *port = NULL;

	id = strtok(value,",");
	ip = strtok(NULL,",");
	port = strtok(NULL,",");
	assert(id != NULL);
	assert(ip != NULL);
	assert(port != NULL);
	if(check_ifname(dev))
	{
		fprintf(stderr,"not a valid interface name.\n");
		goto Failed;
	}
	if(check_ipv4(ip))
	{
		fprintf(stderr,"Not a valid ip address.\n");
		goto Failed;
	}
	if(get_unsigned16(v_port,port,10))
	{
		fprintf(stderr,"Not a valid port.\n");
		goto Failed;
	}
	if(!is_exist(dev,id))
	{
		assert(inet_pton(AF_INET,ip,nl_ip) > 0);
		strcpy(peerid,id);
		goto Success;
	}else
	{
		fprintf(stderr,"Peer id is dulplicate.%s\n",id);
		goto Failed;
	}
Failed:
	return -1;
Success:
	return 0;
}
//[dev] nw1 peerid PEERID peerip PEERIP  peerpot PORT
int nw_peer_add(int argc, char ** argv)
{	
	struct nw_peer_entry *entry = calloc(1,sizeof(struct nw_peer_entry));
	if(entry == NULL)
		return MEMERR;
	char *dev =NULL;
	char *id = NULL;
	char ip[16];
	u16 port;

	if (argc < 7  || argc > 8)
	{
		fprintf(stderr,"argus are not enough.\n");
		goto FAILED;
	}
	while(argc > 0) 
	{
		if(strcmp(*argv,"peerid") == 0 || strcmp(*argv,"id") == 0 )
		{
			NEXT_ARG();
			id = *argv;
		}else if (strcmp(*argv,"peerip") == 0 || strcmp(*argv,"ip") == 0)
		{
			NEXT_ARG();
			if(check_ipv4(*argv))
			{
				free(entry);
				invarg("invalid ip addr: ",*argv);
			}
			strcpy(ip,*argv);
		}else if(strcmp(*argv,"peerport") == 0 ||strcmp(*argv,"port") == 0)
		{	
			NEXT_ARG();
			if(get_unsigned16(&port,*argv,0))
			{
				free(entry);
				invarg("Invalid \"port\" .",*argv);
			}
		}else
		{
			if(strcmp(*argv,"dev") == 0)
			{
				NEXT_ARG();
			}
			if(dev)
			{
				duparg2("dev",*argv);
				goto FAILED;
			}
			if(check_ifname(*argv) )
			{
				invarg("\"dev\" not a valid ifname", *argv);
				goto FAILED;
			}
			if(check_nw_if(*argv))
			{
				invarg("not a running nw dev.",*argv);
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
	}else if(!is_exist(dev,id))
	{
		strcpy(entry->peerid[0],id);
		inet_pton(AF_INET,ip,&entry->ip[0]);	
		entry->port[0] = port;
		entry->count = 1;
		if(nw_do_add(dev,entry) < 0)
		{
			goto FAILED;
		}else 
		{
			printf("Success!\n");
			goto SUCCESS;
		}
	}else
	{
		fprintf(stderr,"Peer id is dulplicate.\n ");
		goto FAILED;
	}
FAILED:
	free(entry);
	return -1;
SUCCESS:
	free(entry);
	return 0;
}
bool is_exist(const char *dev,char *id)
{
	int ret;
	assert(dev != NULL);
	assert(id != NULL);
	int i;
	struct nw_peer_entry *entry = calloc(1,sizeof(struct nw_peer_entry));
	if((ret = nw_do_peer_list(dev,entry)) < 0)
	{
		goto failed;
	}
	if(entry->head.type == NW_OPER_PEER )
	{

		for(i = 0 ; i < entry->count ; i++)
		{
			if(strcmp(entry->peerid[i],id) == 0)
			{
				goto success;
			}
		}
	}
	goto failed;
failed:
	free(entry);
	return false;
success:
	free(entry);
	return true;
}