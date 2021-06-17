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
	char *id = NULL;
	char ip[16];
	u16 port;
	char *port_str = NULL;
	struct nw_peer_entry *entry = (struct nw_peer_entry *)calloc(1,sizeof(struct nw_peer_entry));
	if(entry == NULL)
	{
		fprintf(stderr,"\ncalloc error.");
		return MEMERR;
	}
	if( argc == 7 )
	{
		if( matches(argv[1],"peerid") == 0 && matches(argv[3],"peerip") == 0 && matches(argv[5],"peerport") == 0)
		{	
			dev = argv[0];
			id  = argv[2];
			strcpy(ip , argv[4]);
			port_str  = argv[6];
		}else
		{
			nw_peer_usage();
			goto failed;
		}
	}else if( argc == 8 )//dev DEV peerid ID peerip IP peerport PORT
 	{
		if(matches(argv[0],"dev") == 0 && matches(argv[2],"peerid")== 0 && matches(argv[4],"peerip") == 0 && matches(argv[6],"peerport") == 0)
		{
			dev = argv[1];
			id = argv[3];
			strcpy(ip ,argv[5]);
			port_str = argv[7];
		}else
		{
			nw_peer_usage();
			goto failed;
		}
	}else if(argc == 4) //DEV PEERID PEERIP PEERPORT
	{
		dev = argv[0];
		id = argv[1];
		strcpy(ip , argv[2]);
		port_str = argv[3];
	}else if (argc == 5)//dev DEV ID IP PORT or DEV peer ID IP PORT
	{
		if(matches(argv[0],"dev") == 0)
		{
			dev = argv[1];
			id = argv[2];
			strcpy(ip,argv[3]);
			port_str = argv[4];
		}else if( matches(argv[1],"peer") == 0)
		{
			dev  = argv[0];
			id = argv[2];
			strcpy(ip,argv[3]);
			port_str = argv[4];
		}else
		{
			nw_peer_usage();
			goto failed;
		}
	}else if (argc == 6)//dev DEV peer ID IP PORT 
	{
		if(matches(argv[0],"dev") == 0 && (matches(argv[2],"peer") == 0 ||matches(argv[2],"peerid") == 0))
		{
			dev = argv[1];
			id = argv[3];
			strcpy(ip,argv[4]);
			port_str = argv[5];
		}else
		{
			nw_peer_usage();
			goto failed;
		}
	}
	else if(argc == 3) //DEV ID IP
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
		nw_peer_usage();
		goto failed;
	}
	if(!port_str)
	{
		port = 0;
	}else if (get_unsigned16(&port,port_str,0))
	{
		fprintf(stderr,"Error:[ %s ]is out of range[ 0-65535 ].\n",port_str);
		goto failed;
	}	
	if(!dev)
	{
		fprintf(stderr,"Error:not enough information:\"dev\" argument is required.\n");
		goto failed;
	}
	if(is_exist(dev,id))
	{
		strcpy(entry->peerid[0],id);
		if(check_ipv4(ip))
		{
			fprintf(stderr,"Error:%s is not valid peer ip address.\n",ip);
			goto failed;
		}
		inet_pton(AF_INET,ip,&entry->ip[0]);
		entry->port[0] = port;
		if(nw_do_change(dev,entry) < 0)
		{
			fprintf(stderr,"Error:%s change peer failed.\n",dev);
			goto failed;
		}
		printf("Success!\n");
	}else
	{
		fprintf(stderr,"Error: Peerid \'%s\' of \'%s\' is not exist.\n",id,dev);
		goto failed;
	}
	goto succeed;
succeed:
	free(entry);
	return 0;
failed:
	free(entry);
	return -1;
}

//dev nw1 peerid p1,p2,p3
int nw_peer_del(int argc, char ** argv)
{
	struct nw_peer_entry *entry = calloc(1,sizeof(struct nw_peer_entry));
	int i = 0;
	char *cur = NULL;
	char *dev = NULL;
	int ret;
	if(argc < 3 || argc > 4)
	{
		fprintf(stderr,"Error:number of argc [ %d ]is unexpected.\n",argc);
		return -1;
	}
	while(argc > 0)
	{
		if(strcmp(*argv,"peerid") == 0)
		{
			NEXT_ARG();
			if(strlen(*argv) == 0)
			{
				goto failed;
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
				duparg2("dev",*argv);
				goto failed;
			}
			if(check_ifname(*argv))
			{
				invarg("\"dev\" not a valid ifname.\n", *argv);
				goto failed;
			}
			if(check_nw_if(*argv))
			{
				invarg("not a running nw dev.\n",*argv);
				goto failed;
			}
			dev = *argv;
		}
		argc--;	
		argv++;
	}
	if(!dev)
	{
		fprintf(stderr,"Error:\"dev\" argument is required.\n");
		goto failed;
	}
	if(entry->count)
	{
		if( (ret = nw_do_del(dev,entry)) == 0)
		{
			fprintf(stderr,"Success!\n");
		}else
		{
			fprintf(stderr,"Failed.\n");
		}	
	}
	goto success;
success:
	free(entry);
	return 0;
failed:
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
		fprintf(stderr,"Error:Option with dev %s is not valid.\n",dev);
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
	if( argc == 7 )
	{
		if( matches(argv[1],"peerid") == 0 && matches(argv[3],"peerip")==0 && matches(argv[5],"peerport") == 0)
		{	
			dev = argv[0];
			id  = argv[2];
			strcpy(ip , argv[4]);
			port_str  = argv[6];
		}else
		{
			nw_peer_usage();
			goto params_error;
		}
	}else if( argc == 8 )
	{
		if(matches(argv[0],"dev") == 0 && matches(argv[2],"peerid")== 0 && matches(argv[4],"peerip") == 0 && matches(argv[6],"peerport") == 0)
		{
			dev = argv[1];
			id = argv[3];
			strcpy(ip ,argv[5]);
			port_str = argv[7];
		}else
		{
			nw_peer_usage();
			goto params_error;
		}
	}else if(argc == 4) //DEV PEERID PEERIP PEERPORT
	{
		dev = argv[0];
		id = argv[1];
		strcpy(ip , argv[2]);
		port_str = argv[3];
	}else if (argc == 5)//dev DEV ID IP PORT
	{
		if(matches(argv[0],"dev") == 0){
			dev = argv[1];
			id = argv[2];
			strcpy(ip ,argv[3]);
			port_str = argv[4];
		}
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
		fprintf(stderr,"Error: [ %d ]  unexpected param nums.\n",argc);
		goto param_nums_err;
	}
	if(!port_str)
	{
		port = 0;
	}else if (get_unsigned16(&port,port_str,0))
	{
		fprintf(stderr,"Error:[ %s ] is not a valid port num.\n ",port_str);
		goto port_validate_failed;
	}	
	//add procedure.
	if(!dev)
	{
		fprintf(stderr,"Error:not enough information:\"dev\" argument is required.\n");
		goto dev_is_required;
	}else if (check_nw_if(dev))
	{
		fprintf(stderr,"Error:[ %s ] is not running.\n",dev);
		goto dev_is_exist;
	}
	else if(!is_exist(dev,id))
	{
		strcpy(entry->peerid[0],id);
		inet_pton(AF_INET,ip,&entry->ip[0]);	
		entry->port[0] = port;
		entry->count = 1;
		if(nw_do_add(dev,entry) < 0)
		{
			goto peer_not_exist;
		}else 
		{
			printf("Success!\n");
			goto success;
		}
	}else
	{
		fprintf(stderr,"Error :peerid [%s] is dulplicate.\n",id);
		goto peerid_is_dulplicate;
	}
port_validate_failed:
peerid_is_dulplicate:
dev_is_required:
dev_is_exist:
peer_not_exist:
params_error:
param_nums_err:
	free(entry);
	return -1;
success:
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
	if(entry == NULL)
	{
		return -1;
	}
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