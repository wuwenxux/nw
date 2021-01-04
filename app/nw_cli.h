#ifndef _NW_CLI_H
#define _NW_CLI_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>


#define u16 unsigned short
#define u32 unsigned int
#define u64 uint64_t

#define MAX_PEERNAME_LENGTH 64		//包括最后的0字符在内。字符串实际长度最大63个字符。
#define MAX_PEER_NUMBER 256

#define NW_OPER	(SIOCDEVPRIVATE + 1)

#define NW_OPER_PEER		0x01		//peer设置和读取
#define NW_OPER_PEERSTATUS	0x02		//peer的状态
#define NW_OPER_TYPE		0x03		//peer类型，服务端/客户端
#define NW_OPER_BIND		0x04		//本端绑定端口
#define NW_OPER_PING		0x05		//PING参数
#define NW_OPER_OTHER		0x06		//其它参数
#define NW_OPER_SELF		0x07		//本端的peerid
#define NW_OPER_DEVSTAT		0x08		//网卡统计数据

#define NW_COMM_READ			0		//读取参数
#define NW_COMM_SET				1		//设置参数
#define NW_COMM_PEER_ADD		2		//增加peer，重复就替换
#define NW_COMM_PEER_CHANGE		3		//修改peer
#define NW_COMM_PEER_DEL		4		//删除peer
#define NW_COMM_PEER_LIST		5		//列出已有peer
#define NW_COMM_PEER_CONNECT	6		//发起peer连接
#define NW_COMM_PEER_CLOSE		7		//关闭peer连接

#define NW_OPER_FAIL		0			//操作失败
#define NW_OPER_SUCCESS		1			//操作成功


struct nw_oper_head
{
	u32 type;			//操作类型ID
	u32 command;		//操作命令ID
	u32 result;			//操作结果。应用传入设置初始值为0。驱动操作成功设置为1。应用通过此值，判断本次操作是否成功。
	char devname[IFNAMSIZ];
};

struct nw_peer_entry
{
	struct nw_oper_head head;
	u32 count;				//peer总数
	char peerid[MAX_PEER_NUMBER][MAX_PEERNAME_LENGTH];
	u32 ip[MAX_PEER_NUMBER];
	u16 port[MAX_PEER_NUMBER];
};

struct nw_peer_status
{
	struct nw_oper_head head;
	u32 count;				//peer总数
	char peerid[MAX_PEER_NUMBER][MAX_PEERNAME_LENGTH];
	u32 ip[MAX_PEER_NUMBER];				//当前连接的peer端的IP地址
	u16 port[MAX_PEER_NUMBER];				//当前连接的peer端的端口
	u32 active[MAX_PEER_NUMBER];			//0-当前已断开，1-当前已连接
	u64 connecttime[MAX_PEER_NUMBER];		//本次连接时刻（从1970年开始的秒数）
	u64 sendpackets[MAX_PEER_NUMBER];
	u64 sendbytes[MAX_PEER_NUMBER];			//字节单位
	u64 sendspeed[MAX_PEER_NUMBER];			//字节单位
	u64 recvpackets[MAX_PEER_NUMBER];
	u64 recvbytes[MAX_PEER_NUMBER];			//字节单位
	u64 recvspeed[MAX_PEER_NUMBER];			//字节单位
};
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
struct nw_type
{
	struct nw_oper_head head;
	u32 mode;			//0-server,1-client
};

struct nw_bind
{
	struct nw_oper_head head;
	u16 port;
};

struct nw_ping
{
	struct nw_oper_head head;
	u32 interval;			//毫秒单位，必须是100毫秒的整数倍
	u32 timeout;			//毫秒单位，必须是100毫秒的整数倍
};

struct nw_other
{
	struct nw_oper_head head;
	//设置的时候，下面某个参数值为0，就是忽略该参数。只设置非0值的参数。
	u32 bufflen;
	u32 maxbufflen;
	u32 queuelen;
	char oneclient[4];			//参数值，yes/no
	char showlog[4];			//参数值，yes/no
	u32 batch;
	u32 idletimeout;			//单位秒
};

struct nw_self
{
	struct nw_oper_head head;
	char peerid[MAX_PEERNAME_LENGTH];
};

struct nw_dev_stat
{
	struct nw_oper_head head;
	u32 up;			//0-down，1-up
	u64 sendpackets;
	u64 senddrops;
	u64 senderrors;
	u64 sendbytes;			//字节单位
	u64 sendspeed;			//字节单位
	u64 recvpackets;
	u64 recvdrops;
	u64 recverrors;
	u64 recvbytes;			//字节单位
	u64 recvspeed;			//字节单位
};
const char* dev_stat_str[] =
{
	"Up",
	"sendpackets",
	"senddrops",
	"senderrors",
	"sendbytes",
	"sendspeed",
	"recvpackets",
	"recvdrops",
	"recverrors",
	"recvbytes",
	"recvspeed",
	NULL,
};
#endif /* _NGMWAN_DEVCTL_H */
