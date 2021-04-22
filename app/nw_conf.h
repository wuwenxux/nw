#ifndef NW_CONF
#define NW_CONF
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
//structures
/**
 * Struct of a conf file(name, configs) 
 * @name:basename of this config file
 * @configs:a conf file has a list of configs
 **/
struct nw_file
{
    char* name;
    struct nw_config *configs;
};
/**
 * A detailed config structure
 * @name: name of the config section.
 * @options:a config with a list of options
 * @next:a conf file has a list of configs
 **/
struct nw_config
{
    char* name;
    struct nw_option *options;
    struct nw_config *next;
};
/**
 * A option of a ngmwanconf file
 * option  key : value
 * @key:index to store the value
 * @value:value of the key to be stored
 * @next:one link of a option list 
 **/
struct nw_option
{
    char *key;
    char *value;
    struct nw_option *next;
};
/**
 * Parse the given file and load it to the structures to process it
 *  Return: a struct nw_file object if success else return NULL.
 * @path:file path
 **/
struct nw_file *file_open(char *path);
/**
 * Return: return the nw_config object if found, else return NULL.
 * @file: the nw_file object
 * @name: config name
 **/
struct nw_config *find_config(struct nw_file *file,const char *name);
/**
 * Find a option with @key in the config
 * Return : nw_option ,else NULL.
 * @name:config name
 * @key:the key of the option to search
 **/
struct nw_option *find_option(struct nw_config *name,const char *key);
//struct nw_list *find_list(struct nw_config *name,const char *name);
/**
 * Load config to set up a dev 
 * Note:ifname of this config is not valid return -1.
 * Return:0 Success , DEV_NOT_FOUND,DEV_IS_RUNNING,IOCTLERR,INVALID_ARG Failed.
 * @dev:ifname
 * @ip:ip addr
 * @mask:netmask
 **/
int nw_setup_dev(const char *dev, char *ip,char *mask);
/**
 * Load config and configure nw_other info.
 * Return:0 Success ,DEV_NOT_FOUND,IOCTLERR,CHECKERR Failed.
 * @config: a ptr of listhead which points to nw_config.
 **/
int nw_setup_other(struct nw_other *o_ptr);
/**
 * Load config and configure nw_peer_entry info.
 * Return: 0 Success , CMDERR,DEV_NOT_FOUND ,MEMERR Failed.
 * @config:a ptr of listhead which points to nw_config.
 **/
int nw_setup_peer(struct nw_peer_entry *e_ptr);
/*util*/
/**
 * Convert  from a u32 value to a client/server str.
 * Return: client/server  if not exist NULL.
 * @mode:1 client 2 server 
 **/

/**
 * Find the value of a option with key
 * Return : value of this option
 * @name:name of the conf
 * @key:key of the option
 **/
char* find_value(struct nw_config *name,const char *key);
/**
 * Close a file
 * Free all allocated structures : options,configs
 * file supposed to be valid ,if not return.
 * @file:the nw_file object to close
 **/
void file_close(struct nw_file **file);
#endif