#ifndef NW_CONF
#define NW_CONF
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
//structures
struct nw_file
{
    char* name;
    struct nw_config *configs;
};
struct nw_config
{
    char *name;
    struct nw_option *options;
    struct nw_config *next;
};
struct nw_option
{
    char *key;
    char *value;
    struct nw_option *next;
};
//funcs
struct nw_file *file_open(char *);
struct nw_config *find_config(struct nw_file *,const char *);
struct nw_option *find_option(struct nw_config *,const char *);
char* find_value(struct nw_config *,const char *);
int conf_check_option(struct nw_config *,char *,const char *);
void file_close(struct nw_file **file);
#endif