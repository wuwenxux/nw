#ifndef NW_CLI_H_
#define NW_CLI_H_
#include "manage.h"
#define NW_MODE_SERVER  2
#define NW_MODE_CLIENT  1
#define NGMWAN_GENL_VERSION_NAME "1.0.0.1"
#define PUT_BAR_T(str)	printf("*****************************************\n"); \
			            printf("*  ngmwan interface: %s                *\n", str);\
			            printf("*****************************************\n");
#define DEF_CONF_FILE  "/etc/config/ngmwan"
#define CONF_NAME_SIZE 20

/*common*/
void nw_dev_show_statistic(char  *);
void nw_dev_show_args(char *);
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
int nw_other_read(const char *,struct nw_other *);
int nw_bind_read(const char *, struct nw_bind *);
int nw_type_read(const char *, struct nw_type *);
int nw_ping_read(const char *, struct nw_ping *);
int nw_self_read(const char *, struct nw_self*);
int nw_self_set(const char * , struct nw_self *);
int nw_other_set(const char *, struct nw_other *);
int nw_type_set(const char *,  struct nw_type *);
int nw_ping_set(const char *,  struct nw_ping *);
int nw_bind_set(const char *,  struct nw_bind *);


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



int nw_do_peer_list(const char *,struct nw_peer_entry *);

/*conf*/
int nw_load_conf(char *path);
int nw_save_conf(int argc, char **argv);
int call_cmd(int argc,char **argv);

/*util*/
const char* mode_str(u32);
int nw_search_if(char *);

/*read*/
void do_read(struct nw_oper_head *head);

#endif/* NW_CLI_H */