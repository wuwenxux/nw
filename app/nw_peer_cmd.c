#include "utils.h"
#include "nw_cli.h"

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
int nw_peer_change(int argc, char **argv)
{
    return 0;
}
int nw_peer_connect(int argc, char ** argv)
{
    return 0;
}
static int nw_peer_ioctl(struct nw_peer_entry *entry)
{
    struct ifreq req;
    int sock,ret;

    memset(&req,0,sizeof(req));
    req.ifr_data = (void *)entry;
    strcpy(req.ifr_name,entry->head.devname);

    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(!sock)
    {
		printf("peer sockert err.\n");
        return -1;
    }
    
    ret = ioctl(sock,NW_OPER,&req);
    if(ret == -1 ) 
	{
		printf("nw %s oper %d error\n",entry->head.devname,entry->head.command);
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
		ret = nw_peer_ioctl(&peer);
		if(ret)
			printf("peer del error.\n");
	}
	return ret;
}
//peerid p1 peerip 192.168.2.1 peerport 82534
int nw_peer_set(int argc, char **argv)
{
	struct nw_peer_entry peer;
	int ret;
	char *p,*q,**err = NULL;
	if(argc != 6)
	{
		/*command error*/
		nw_peer_usage();
	}else
	{
		if(strncmp(argv[0],"peerid",6) != 0 || strncmp(argv[2],"peerip",6) != 0 || strncmp(argv[4],"peerport",8) != 0)
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
		ret = nw_peer_ioctl(&peer);
		if(ret)
		{
			printf("peerip %s  peerport %s",argv[7],argv[8]);
		}			
	}
	return ret;
}
//peerid p1 peerip 192.168.2.1 peerport 82534
 int nw_peer_add(int argc, char ** argv)
{
	struct nw_peer_entry peer;
	int ret;
	char *p,*q,**err = NULL;
	if(argc != 6)
	{
		/*command error*/
		nw_peer_usage();
		return 0;
	}else
	{
		memset(&peer,0,sizeof(struct nw_peer_entry));	
		if(strncmp(argv[0],"peerid",6) != 0 ||strncmp(argv[2],"peerip",6) != 0 ||strncmp(argv[4],"peerport",8) != 0)
		{
			perror("peerip peerport  assumed.");
			return CMDERR;		
		}
		strncpy(peer.peerid[0], argv[1],sizeof(argv[1]));

		inet_pton(AF_INET,argv[3],&peer.ip[0]);
		
		peer.port[0] = (unsigned short) strtoul(argv[5], err, 0);
		peer.count = 1;
		peer.head.type = NW_OPER_PEER;
		peer.head.command = NW_COMM_PEER_ADD;
		ret = nw_peer_ioctl(&peer);
		if(ret)
		{
			printf("peerip : %s ",argv[3]);
			printf("peerpot: %s ",argv[5]);
		}
	}
		return ret;
}
//peerid p1,p2,p3
//type:NW_OPER_PEER command:NW_COMM_PEER_DEL
int nw_peer_list_del(int argc, char **argv)
{
	struct nw_peer_entry peer;
	int ret,i;
	char token[MAX_PEER_NUMBER][MAX_PEERNAME_LENGTH];
	char *p,
		 *save_p =NULL,
		  **err = NULL;
	memset(&peer,0,sizeof(struct nw_peer_entry));
	memset(token,0,sizeof(token));
	peer.count = i;
	peer.head.type =NW_OPER_PEER;
	peer.head.command=NW_COMM_PEER_DEL;
	ret = nw_peer_ioctl(&peer);
	if(ret)
	{
			printf("ioctl err.\n");
	}
	return ret;
}
//nw show dev nw1 peerid 
//type:NW_OPER_PEER command:NW_COMM_PEER_LIST
int nw_peer_show(int argc, char **argv)
{
	return 0;
}