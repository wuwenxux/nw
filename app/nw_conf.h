#ifndef _NW_CONF_H_
#define _NW_CONF_H_
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


/**
 * Structures
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
 * @next:pointer to next config if not null else set pointer null.
 **/
struct nw_config
{
    char* name;
    struct nw_option *options;
    struct nw_config *next;
}conf;

/**
 * A option of a ngmwanconf file
 * option  key : value
 * @key:index to store the value
 * @value:values of the key to be stored
 * @next:one link of a option list 
 **/
struct nw_option
{
    char *key;
    struct nw_value *values;
    struct nw_option *next;
};

/**
 *A value list of nw option.
 *option key : value, value ,values...
 **/
struct nw_value{
    char *string;
    struct nw_value *next;
};

/**
 * Parse the given file and load it into the conf structures.
 * Return: a struct nw_file object if success else return NULL.
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

/**
 * Find a value with @key in the option
 * Return: 0 found,eles -1;
 * @opt:nw_option
 * @key:option key
 **/
int find_value(struct nw_option *opt, const char *key);

/**
 * Load config to set up a dev 
 * Note:ifname of this config is not valid return -1.
 * Return:0 Success , DEV_NOT_FOUND,DEV_IS_RUNNING,IOCTLERR,INVALID_ARG Failed.
 * @dev:ifname
 * @ip:ip addr
 * @mask:netmask
 * @mode:client/server
 **/
int nw_setup_dev(const char *dev, char *ip,char *mask);

/**
 * Close a file
 * Free all allocated structures : options,configs
 * file supposed to be valid ,if not return.
 * @file:the nw_file object to close
 **/
void file_close(struct nw_file **file);

/**
 * Remove all nw dev
 **/
static void nw_clear();

/**
 * Load a config to a dev
 * Return :0 SUCCESS -1 FAIL
 * @config:import the config object to set up a ngmwan interface.
 */
static int nw_load_dev(struct nw_config *config);

/**
 * Load conf
 * Return 0 success , -1 fail
 * @path: path to the conf file 
 **/ 
int nw_load_conf(char *path);

/**
 * Save one nw dev interface
 * Return:0 success , -1 failure
 *@fp:export one dev conf interface to fp
 *@dev:conf of dev
 **/
static int nw_save_dev(FILE *fp,const char *dev);
#endif