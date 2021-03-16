#ifndef NW_CLI_H_
#define NW_CLI_H_
#include "manage.h"
/*common*/
void nw_dev_show_statistic(char  *);
void nw_dev_show_args(char *);
/*ioctl*/
int nw_ioctl(struct nw_oper_head *);
/*show*/
int nw_dev_show(int ,char **);
int nw_dev_set(int , char **);
int nw_dev_bindport(int ,char **);

int nw_search_if(char *);
/*help*/
void nw_usage();
void nw_peer_usage();
void nw_self_usage();
void nw_mode_usage();
void nw_set_usage();
/*other type ping */
int nw_other_set(const char *, struct nw_other *);
int nw_type_set(const char *,  struct nw_type *);
int nw_ping_set(const char *,  struct nw_ping *);
int nw_bind_set(const char *,  struct nw_bind *);
int nw_other_read(const char *,struct nw_other *);
int nw_bind_read(const char *, struct nw_bind *);
int nw_type_read(const char *, struct nw_type *);
int nw_ping_read(const char *, struct nw_ping *);

/*connect*/
int nw_dev_connect(int argc, char **argv);
int nw_dev_close(int argc,char **argv);
/*peer cmd*/
int nw_do_change(const char *,struct nw_peer_entry *);
int nw_do_add(const char *, struct nw_peer_entry *);
int nw_do_del(const char *, struct nw_peer_entry *);
int nw_do_peer_list(const char *,struct nw_peer_entry *);

/*util*/
const char* mode_str(nw_mode_t );

/*read*/

void do_read(struct nw_oper_head *head);

#endif/* NW_CLI_H */