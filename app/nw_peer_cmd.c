#include "utils.h"
#include "nw_cli.h"

static int nw_ioctl(struct nw_oper_head *head)
{
    struct ifreq req;
    int sock,ret;

    memset(&req,0,sizeof(req));
    req.ifr_data = (void *)head;
    strcpy(req.ifr_name,head->devname);
	head->result=0;

    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(!sock)
    {
		printf("peer sockert err.\n");
        return -1;
    }
    
    ret = ioctl(sock,NW_OPER,&req);
    if(ret == -1 ) 
	{
		printf("nw %s oper %d error\n",head->devname,head->command);
	}
    close(sock);
    return ret;
}
//nw del dev nw1 peerid p1  
int nw_peer_del(int argc, char * argv[])
{
	struct nw_peer_entry peer;
	int ret,i;
	if(argc != 6)
	{
		/*command error*/
		nw_usage();
		return 0;
	}else
	{
		memset(&peer,0,sizeof(struct nw_peer_entry));
		peer.head.type = NW_OPER_PEER;
		peer.head.command = NW_COMM_PEER_DEL;
		peer.count = 1;
		ret = nw_ioctl(&peer.head);
		if(ret)
			printf("peer del error.\n");
	}
	return ret;
}
//nw change dev nw1 peerid p1 peerip 192.168.2.1 peerport 82534
int nw_peer_change(int argc, char *argv[])
{
	struct nw_peer_entry peer;
	int ret;
	char *p,*q,**err = NULL;
	if(argc != 10)
	{
		/*command error*/
		nw_peer_usage();
		ret = 0;
		return ret ;
	}else
	{
		if(strncmp(argv[4],"peerid",6) != 0 || strncmp(argv[6],"peerip",6) != 0 || strncmp(argv[8],"peerport",8) != 0)
		{	
			return CMDERR;
		}
		memset(&peer,0,sizeof(struct nw_peer_entry));	
		strncpy(peer.peerid[0], argv[5],sizeof(argv[5]));
		inet_pton(AF_INET,argv[7],&peer.ip[0]);
		peer.port[0] =  (unsigned short) strtoul(argv[9], err, 0);
		peer.count = 1;
		peer.head.type = NW_OPER_PEER;
		peer.head.command = NW_COMM_PEER_CHANGE;
		ret = nw_ioctl(&peer.head);
		if(ret)
		{
			printf("peerip %s  peerport %s",argv[7],argv[8]);

		}			
	}
	return ret;
}
//nw add dev nw1 peerid p1 peerip 192.168.2.1 peerport 82534
 int nw_peer_add(int argc, char *argv[])
{
	struct nw_peer_entry peer;
	int ret;
	char *p,*q,**err = NULL;
	if(argc != 10)
	{
		/*command error*/
		nw_peer_usage();
		return 0;
	}else
	{
		memset(&peer,0,sizeof(struct nw_peer_entry));	
		if(strncmp(argv[4],"peerid",6) != 0 ||strncmp(argv[6],"peerip",6) != 0 ||strncmp(argv[8],"peerport",8) != 0)
		{
			perror("peerip peerport  assumed.");
			return CMDERR;		
		}
		strncpy(peer.peerid[0], argv[5],sizeof(argv[5]));

		inet_pton(AF_INET,argv[7],&peer.ip[0]);
		
		peer.port[0] = (unsigned short) strtoul(argv[9], err, 0);
		peer.count = 1;
		peer.head.type = NW_OPER_PEER;
		peer.head.command = NW_COMM_PEER_ADD;
		ret = nw_ioctl(&peer.head);
		if(ret)
		{
			printf("peerip : %s ",argv[7]);
			printf("peerpot: %s ",argv[9]);
		}
	}
		return ret;
}
//nw set dev nw1 del peerid p1,p2,p3 
 int nw_peer_list_del(int argc,char *argv[])
{
	struct nw_peer_entry peer;
	int ret,i;
	char token[MAX_PEER_NUMBER][MAX_PEERNAME_LENGTH];
	char *p,
		 *save_p =NULL,
		  **err = NULL;
	memset(&peer,0,sizeof(struct nw_peer_entry));
	memset(token,0,sizeof(token));
	if(argc !=6 )
	{
		nw_peer_usage();
		ret = 0;
		return ret;
	}
	else
	{
		strcpy(peer.head.devname,argv[3]);
		/* argv[5] p1,p2,p3*/
		if(argv[5] == NULL)
		{
				printf("command err.");
		}
		else
		{	
			for(i = 0,p = argv[5];(p = strtok_r(p,",",&save_p) != NULL);p = NULL,i++)
			{
				if(i > MAX_PEER_NUMBER -1)
				{
						printf("command error.too many peers");
						return -1;
				}
				else
				{
					strcpy(token[i],p);
					strcpy(peer.peerid[i],token[i]);
				}
			}
		}
		peer.count = i;
		peer.head.type =NW_OPER_PEER;
		peer.head.command=NW_COMM_PEER_DEL;
		ret = nw_ioctl(&peer.head);
		if(ret)
		{
			printf("ioctl err.\n");
		}
	}
	return ret;
}
//dev nw1 peer  
//NW_COMM_PEER_LIST
 int nw_peer_show(int argc, char *argv[])
{	
	struct nw_peer_entry peer;
	int ret;

	if(argc < 5)
	{
		nw_peer_usage();
		return 0;
	}
	else
	{
		memset(&peer,0,sizeof(struct nw_peer_entry));
		peer.head.type = NW_OPER_PEER;
		peer.head.command = NW_COMM_PEER_LIST;
		ret = nw_ioctl(&peer.head);
		if(ret)
		{
			printf("ioctl err.");
		}	
		return ret;
	}
}