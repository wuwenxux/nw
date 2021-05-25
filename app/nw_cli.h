#ifndef NW_CLI_H_
#define NW_CLI_H_
#include "manage.h"
#include <stdbool.h>
#define NW_MODE_SERVER  2
#define NW_MODE_CLIENT  1
#define NGMWAN_GENL_VERSION_NAME "1.0.0.1"
#define PUT_BAR_T(str)	printf("******************************************\n"); \
			            printf("*  ngmwan interface: %s                 *\n", str);\
			            printf("******************************************\n");
#define DEFAULT_CONF_FILE  "./ngmwan"
#define DEFAULT_SAVE_FILE  "./save_ngmwan"
#define CONF_NAME_SIZE 20

/*common*/
void nw_dev_show_statistic(char  *);
void nw_dev_show_args(char *);
int  nw_mptcp_set(char *dev ,bool yes_no);
/*ioctl*/
/**
 * a interface between kernel and userspace to control our nw device.
 *Return :0 Success -1 Failed
 *@head:refer to struct nw_oper_head  
 **/
int nw_ioctl(struct nw_oper_head *head);
/*show*/
/**
 *show the dev params : other ,bind ,peer,ping
 *Return: 0 Success -1 Failed
 *@argc:num of argus
 *@argv:argu list
 **/
int nw_dev_show(int argc,char **argv);
/*set the dev params: other, bind,peer,ping*/
int nw_dev_set(int , char **);

/*help*/
void nw_usage();
void nw_peer_usage();
void nw_self_usage();
void nw_mode_usage();
void nw_set_usage();

/*other,type,ping,bind */
/**
 * These funcs below just do two thing :read and write configure of the ngmwan interface by ioctl.
 * All values are supposed valid input ,if not  return -1.
 * Return: 0 Success ,-1 Failed
 * @dev: the device of this ngmwan interface
 * @other: the nw_other values of nw interface dev(refer to "mangage.h")
 * @type:type of the nw interface.
 * @ping:interval and timeout value.
 * @self:ownid,char[16]value.
 * @bind: u16 value port of the nw interface dev.
 **/
int nw_other_read(const char *dev,struct nw_other *other);
int nw_bind_read(const  char *dev, struct nw_bind *bind);
int nw_type_read(const  char *dev, struct nw_type *type);
int nw_ping_read(const  char *dev, struct nw_ping *ping);
int nw_self_read(const  char *dev, struct nw_self *self);
int nw_self_set(const   char *dev, struct nw_self *self);
int nw_other_set(const  char *dev, struct nw_other *other);
int nw_type_set(const   char *dev, struct nw_type *type);
int nw_ping_set(const   char *dev, struct nw_ping *ping);
int nw_bind_set(const   char *dev, struct nw_bind *bind);
int nw_dhcp_set(const 	char *dev, struct nw_dhcp *dchp);
int nw_dhcp_read(const 	char *dev, struct nw_dhcp *dhcp);


/*peer*/
/**
 * Parse user input to a nw_peer_entry object
 * Return: 0 Success -1 Failed
 * @argc:number of argvs
 * @argv:params list
 **/
int nw_peer_change(int argc, char **argv);
/**
 * A ioctl request to change the peer info in kernel
 * Return: 0 Success -1 Failed
 * @argc:number of argvs
 * @argv:params list
 **/
int nw_do_change(const char *,struct nw_peer_entry *);
/**
 * Parse user input to a nw_peer_entry object
 * Return:0 Success -1 Failed
 * @argc:number of argus
 * @argv:argu params list
 **/
int nw_peer_del(int argc, char ** argv);
/**
 * A ioctl request to del a peer
 * Return:0 Success -1 Failed
 * @argc:number of argus
 * @argv:argu params list
 **/
int nw_do_del(const char *, struct nw_peer_entry *);
/**
 * Add a peer from a nw conf opt
 * Return :a ptr to struct nw_peer_entry *
 * @dev:name of this dev
 * @val:peer info: str id,ip,port
 * @peerid:id of this peer to return
 * @nl_ip:netorder ip addr  to return
 * @v_port:port number of peer
 **/
int check_opt_peer(const char *dev, char *value,char *peerid,u32 *nl_ip, u16 *v_port);
/**
 * Parse argv to a nw_peer_entry object
 * Return:0 Success -1 Failed
 * @argc:number of argus
 * @argv:argu params list
 **/
int nw_peer_add(int argc, char ** argv);
/**
 * A ioctl request to add a peer
 * Return:0 Success -1 Failed
 * @argc:number of argus
 * @argv:argu params list
 **/
int nw_do_add(const char *, struct nw_peer_entry *);
/**
 * Check a peer id  is existed
 * Return : true  Exist , false Not Exist
 * @entry: entry to the peers.
 * @id: a peer with id.
 **/
bool is_exist(const char *dev,char *id);
/**
 * Show the peers of nw device
 * @dev and @entry are valid ,if not return -1.
 * Return: 0 SUCCESS , -1 FAILED
 * @dev:owner of the peers.
 * @entry:entry of peers.
 **/ 
int nw_do_peer_list(const char *dev,struct nw_peer_entry *entry);
/*connect*/
/**
 * A ioctl request to connect a peer 
 * Return:0 Success -1 Failed
 * @argc:number of argus
 * @argv:argu params list
 **/
int nw_dev_connect(int argc, char **argv);
/**
 * A ioctl request to close a peer
 * Return:0 Success -1 Failed
 * @argc:number of argus
 * @argv:argu params list
 **/
int nw_dev_close(int argc,char **argv);

/*conf*/
/**
 * Load a a conf file with openwrt configuration format.
 * A valid file path is expected. if not return -1.
 * Return : 0 SUCCESS ,-1 FAILED
 * @path:the path to the file ,filename is included.
 **/
int nw_load_conf(char *path);

/**
 * Save a nw dev  to a config file which is compatiable with openwrt configuration format
 * Dev shoudld be a valid ngmwan interface. 
 * Return :only dev is enough.
 * @dev:name of the device, which can be found throug ip link.
 **/
int nw_save_conf(char *path);
/**
 * Convert a mode value to char *
 * Note: only "client" and "server" are returned.
 * Return : 1 client, 2 server
 **/
const char* mode_str(u32 mode);
/**
 * Convert a mode string to mode value
 * Note: only
 **/
u32 str_mode(const char *mode);
/**
 * Search a running ngmwan interface by ip link
 * Return : 0 Success -1 Failed
 * @dev:check whether dev is a running ngmwan interface.
 **/
int nw_search_if(char *dev);

/*read*/
/**
 * Read the configure of a nw dev struct(nw_other,nw_bind,nw_ping,nw_peer)with name DEV.
 * Return : no return
 * @head:a oper_head which has dev and type of nw_struct(nw_other,nw_bind,nw_ping,nw_peer).   
 **/
void do_read(struct nw_oper_head *head);

#endif/* NW_CLI_H */