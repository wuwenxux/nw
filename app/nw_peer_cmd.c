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
	u16  peerport;
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
				duparg2(dev,*argv);
			}
			if(check_ifname(*argv) )
			{
				free(entry);
				invarg("\"dev\" not a valid ifname", *argv);
				return -1;
			}
			if(check_nw_if(*argv))
			{
				free(entry);
				invarg("not a running nw dev.",*argv);
				return -1;
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
				for(cur = strtok(*argv,","); cur != NULL; cur = strtok(NULL,","), i++)
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
				return -1;
			}
			if(check_ifname(*argv))
			{
				free(entry);
				invarg("\"dev\" not a valid ifname", *argv);
				return -1;
			}
			if(check_nw_if(*argv))
			{
				free(entry);
				invarg("not a running nw dev.",*argv);
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
		goto FAILED;
	}
	if(entry->count)
	{
		if(nw_do_del(dev,entry) < 0)
		{
			goto FAILED;
		}
		else
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
		fprintf(stderr,"Option with dev %s is not valid.\n",dev);
		goto Failed;
	}
	//convert ip to u32 form
	if(check_ipv4(ip)||inet_pton(AF_INET,ip,nl_ip)<1)
	{
		fprintf(stderr,"Option with ip %s is not valid.\n",ip);
		goto Failed;
	}
	if(get_unsigned16(v_port,port,10))
	{
		fprintf(stderr,"%s is not valid.\n",port);
		goto Failed;
	}
	strcpy(peerid,id);
	goto Success;
Failed:
	return -1;
Success:
	return 0;
}
//[dev] nw1 peerid PEERID peerip PEERIP  peerport PORT
int nw_peer_add(int argc, char ** argv)
{	
	struct nw_peer_entry *entry = calloc(1,sizeof(struct nw_peer_entry));
	if(entry == NULL)
		return MEMERR;
	char *dev = NULL;
	char *id = NULL;
	char ip[16];
	char *port_str = NULL;
	u16 port ;
	if(argc == 7 )
	{
		dev = argv[0];
		id  = argv[2];
		strcpy(ip , argv[4]);
		port_str  = argv[6];
	}else if(argc == 8 )
	{
		dev = argv[1];
		id = argv[3];
		strcpy(ip ,argv[5]);
		port_str = argv[7];
	}else if(argc == 4) //DEV PEERID PEERIP PEERPORT
	{
		dev = argv[0];
		id = argv[1];
		strcpy(ip , argv[2]);
		port_str = argv[3];
	}else if (argc == 5)//dev DEV ID IP PORT
	{
		dev = argv[1];
		id = argv[2];
		strcpy(ip ,argv[3]);
		port_str = argv[4];
	}else if(argc == 3) //DEV ID IP
	{
		dev = argv[0];
		id = argv[1];
		strcpy(ip,argv[2]);
	}else if (argc == 2) // DEV ID
	{
		dev = argv[0];
		id = argv[1];
		strcpy(ip,"0.0.0.0");
	}else 
	{
		fprintf(stderr,"Argc :[%d] can not be specified.\n",argc);
	}
	
	if(!port_str)
	{
		port = 0;
	}else if( get_unsigned16(&port,port_str,0))
	{
		fprintf(stderr,"%s is not a valid port num.",port_str);
		goto FAILED;
	}	
	//add procedure.
	if(!dev)
	{
		fprintf(stderr,"Not enough information:\"dev\" argument is required.\n");
		goto FAILED;
	}else if (check_nw_if(dev))
	{
		fprintf(stderr,"%s is not running.\n",dev);
		goto FAILED;
	}
	else if(!is_exist(dev,id))
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
		fprintf(stderr,"Peerid: %s is dulplicate.\n",id);
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