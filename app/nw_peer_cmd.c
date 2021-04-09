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
		fprintf(stderr,"Num of argc is not valid.\n");
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
	if(strlen(peerid) >0)
	{
		strcpy(entry->peerid[0],peerid);
		inet_pton(AF_INET,peerip,&entry->ip[0]);
		entry->port[0] = peerport;
		if(nw_do_change(dev,entry) < 0)
		{
			goto FAILED;
		}else
		{
			printf("Success!\n");
		}
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

//[dev] nw1 peerid PEERID peerip PEERIP  peerpot PORT
int nw_peer_add(int argc, char ** argv)
{	

	struct nw_peer_entry *entry = calloc(1,sizeof(struct nw_peer_entry));
	if(entry == NULL)
		return MEMERR;
	char *dev =NULL;
	char *id = NULL;
	char *ip = NULL;
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
			ip = *argv;
		}else if(strcmp(*argv,"peerport") == 0 ||strcmp(*argv,"port") == 0)
		{	
			NEXT_ARG();
			if(get_unsigned16(&port,*argv,0))
			{
				free(entry);
				invarg("Invalid \"port\" .",*argv);
			}
		}else{
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
	}else if(strlen(id))
	{
		strcpy(entry->peerid[0],id);
		inet_pton(AF_INET,ip,&entry->ip[0]);		
		entry->port[0] = port;
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
		fprintf(stderr,"Argus like peerid ip  port are required.\n ");
		goto FAILED;
	}
FAILED:
	free(entry);
	return -1;
SUCCESS:
	free(entry);
	return 0;
}