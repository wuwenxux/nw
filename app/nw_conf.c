#include "nw_cli.h"
#include "nw_err.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "nw_conf.h"
#define trim_str "\'\""
/**
 *Config a ip mask into a dev by .
 *Return : -1 Failed , 0 Success
 * @name : name of the dev.
 * @ip: netorder format of ip or mask.
 **/
static int set_ip_using(const char *name, unsigned long ip);
/**
 * Add a configuration of ngmwan and return nw_config object,like:config interface 'nwconf1';
 * @path and @file are supposed to be valid ,return NULL,if either of them is NULL.
 * Return:@nw_config object
 * @file:a nw_file to load in. 
 * @path:path to the file.
 **/
static struct nw_config* add_config(struct nw_file *file,const char *path);
/**
 * Add a key value pair option into nw_option object from nw_config;
 * @key ,@value must be valid  if either of them is NULL, return NULL.
 * Return:a nw_option object ,NULL if  operation failed or invalid input(either of config,key,value is NULL).
 * @config:the config interface.
 * @key:key of the option
 * @value:value of the option
 **/
static struct nw_option* add_option(struct nw_config *config,const char *key,const char *value);
/**
 * Find a nw_config from the nw_file file
 * @file and @name should be valid input, if NULL or invalid return NULL. 
 * Return:A nw_config listhead object.
 * @file:a nw_file to load 
 * @name:name of this nw_config
 **/ 
struct nw_config* find_config(struct nw_file *file,const char *name);
/**
 * Find the value of a nw_option
 * Return : the value of a nw_option,NULL if not exist
 * @name:name of the config,if NULL return NULL
 * @key:key of the option,if NULL return NULL
 **/
char* find_value(struct nw_config *name,const char *key);
/**
 * Find a option in the nw_conf object.
 * @name and @key are supposed to be valid ,if not ,return NULL.
 * Return:a nw_option  if found, else NULL.
 * @name:name of this config
 * @key:key of the option name
 **/
struct nw_option* find_option(struct nw_config *name,const char *key);

struct nw_file *file_open(char *path)
{
	char *word = NULL;
	char *conf_name = NULL; 
	char *inface = NULL;
	char *key,*val;
	char  line[BUFLEN];
	FILE *confFile;
	struct nw_file *file = NULL;
	struct nw_config *thisConfig = NULL;

	if((confFile = fopen(path,"r"))== NULL)
	{
		return NULL;
	}
	//file malloc 
	file = malloc(sizeof(struct nw_file));
	file->name = malloc(strlen(path)+1);
	file->configs = NULL;
 
	//assignment of file->name
	memcpy(file->name,path,strlen(path)+1);

	while(fgets(line,BUFLEN,confFile) != NULL)
	{
		blank_del(line);
		word = strtok(line," ");
		if(word == NULL || *word == '#')
		{
			continue;
		}else if(strcmp(word,"config")== 0)
		{
		   inface = strtok(NULL," ");
		   if(inface == NULL)
		   {
				continue;   
		   }
		   else if(strcmp(inface,"interface") == 0)
		   {
				conf_name = strtok(NULL," ");
			 	assert(conf_name != NULL);
			//	printf("conf name :%s",conf_name);
			 	trim(conf_name,trim_str);
				thisConfig = add_config(file,conf_name);
		   }else
		   {
			   fprintf(stderr,"interface is expected.\n");
		   		goto EXIT;
		   }
		}else if(strcmp(word,"option")== 0)
		{
			key = strtok(NULL," ");
		    val = strtok(NULL," ");
		   	assert(key != NULL);
			assert(val != NULL);
			trim(val,trim_str);
		 	add_option(thisConfig,key,val);
		}
	}
		goto EXIT;
EXIT:
    fclose(confFile);
	return file;
}
int nw_load_conf(char *path)
{
	struct nw_file *file  = NULL;
	struct nw_config *thisConf = NULL;
	struct nw_option *thisOpt = NULL;

	if((file = file_open(path))== NULL)
	{
		fprintf(stderr,"Can't open path %s",path);
		return FILE_NOT_FOUND;
	}
	thisConf = file->configs;
	while(thisConf)
	{
		printf("%s\n",thisConf->name);
		thisOpt = thisConf->options;
		while(thisOpt)
		{
			printf("%s :%s \n",thisOpt->key,thisOpt->value);
			thisOpt = thisOpt->next;
		}
		printf("\n");
		thisConf = thisConf->next;
	}
	file_close(&file);
	return 0;
}
int nw_save_conf(const char *dev)
{
	//struct nw_other *other;
	//nw_other_save(other);
	
	return 0;
}
static int set_ip_using(const char *name, unsigned long ip)
{
    struct ifreq ifr;
    struct sockaddr_in sin;
	int sock;
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(!sock)
	{
		fprintf(stderr,"set ip sock err.\n");
		return -1;
	}
    strncpy(ifr.ifr_name, name, IFNAMSIZ);
    memset(&sin, 0, sizeof(struct sockaddr));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ip;
	memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
    
	if (ioctl(sock, SIOCSIFADDR, &ifr) < 0)
	{
		fprintf(stderr,"set ip ioctl err.\n");
		return -1;
	}
    return 0;
}
int nw_setup_dev(struct nw_config *conf)
{
	char *dev = NULL;
	int ret;
	char dev_cmd[50];
	char ip_v4[16];
	unsigned long ip;
	char mask_str[16];
	unsigned long nmask;
	assert(conf != NULL);
	
	if((dev = find_value(conf,"ifname")) == NULL)
	{
		fprintf(stderr,"ifname is required.\n");
		return DEV_NOT_FOUND;
	}
	if(check_nw_if(dev))
	{
		fprintf(stderr,"device %s is running,either change a new ifname or remove this config\n",dev);
		return DEV_IS_RUNNING;
	}
	assert(dev!= NULL);
	sprintf(dev_cmd,"sudo ip link add %s type ngmwan",dev);
	assert(find_value(conf,"ipaddr") != NULL);
	strcpy(ip_v4 ,find_value(conf,"ipaddr"));
	if(check_ipv4(ip_v4))
	{
		fprintf(stderr,"Invalid ipv4 addr.\n");
		return -1;
	}
	if(inet_pton(AF_INET,ip_v4,&ip) <= 0)
	{
		fprintf(stderr,"Invalid conversion from char * to netorder\n");
		return -1;
	}
	assert(find_value(conf,"mask") != NULL);
	strcpy(mask_str,find_value(conf,"mask"));
	if(check_netmask(mask_str))
	{
		fprintf(stderr,"invalid net mask.\n");
		return -1;
	}
	if(inet_pton(AF_INET,mask_str,&nmask) <= 0)
	{
		fprintf(stderr,"Invalid conversion from char * to netorder\n");
		return -1;
	}
	//sudo ip link add dev %s type ngmwan
	system(dev_cmd);
	ret = set_ip_using(dev,ip);
	if(ret < 0)
	{
		return -1;
	} 
	ret =set_ip_using(dev,nmask);
	if(ret < 0)
	{
		return -1;
	}
	return 0;
}
static struct nw_config* add_config(struct nw_file *file,const char *name)
{
	struct nw_config *prevConfig = NULL;
	struct nw_config *thisConfig = NULL;
	
	assert(file != NULL);
	assert(name != NULL);
	if(file == NULL || name == NULL)
	{
		return NULL;
	}
	if((thisConfig = find_config(file,name)) != NULL)
	{
		return thisConfig;
	}
	
	thisConfig = malloc(sizeof(struct nw_config));
	thisConfig->name = malloc(strlen(name)+1);
	memcpy(thisConfig->name,name,strlen(name)+1);

	thisConfig->next 	= NULL;
	thisConfig->options = NULL;

	prevConfig = file->configs;
	//config list is empty 
	
	if(prevConfig == NULL)
	{
		file->configs = thisConfig;
		return thisConfig;
	}
	//iterate this config list
	while(prevConfig && prevConfig->next)
	{
		prevConfig = prevConfig->next;
	}
	prevConfig->next = thisConfig;
	return thisConfig;
}
static struct nw_option* add_option(struct nw_config *conf,const char *key,const char *val)
{
	struct nw_option *thisOpt = NULL;
	struct nw_option *prevOpt = NULL;

	if((thisOpt = find_option(conf,key)) != NULL)
	{
		//overwrite the value
		thisOpt->value = realloc(thisOpt->value,strlen(val)+1);
		//printf("%s find opt.\n",thisOpt->value);
		memcpy(thisOpt->value,val,strlen(val)+1);
		return thisOpt;
	}
	thisOpt = malloc(sizeof(struct nw_option));
	
	//key assignment 
	thisOpt->key = malloc(strlen(key)+1);
	memcpy(thisOpt->key,key,strlen(key)+1);
	//value assignment
	thisOpt->value = malloc(strlen(val)+1);
	memcpy(thisOpt->value,val,strlen(val)+1);
	
	thisOpt->next = NULL;

	//first opt
	prevOpt = conf->options;

	if(prevOpt == NULL)
	{
		conf->options = thisOpt;
		return thisOpt;
	}
	while(prevOpt && prevOpt->next)
	{
		prevOpt = prevOpt->next;
	}
	//add to the opts list
	prevOpt->next = thisOpt;
	return thisOpt;

}
struct nw_config* find_config(struct nw_file *file,const char *name)
{
	struct nw_config *thisConf= NULL;
	if(file == NULL || name == NULL )
		return NULL;
	//first conf
	thisConf = file->configs;

	while(thisConf)
	{
		if(strcmp(thisConf->name,name) == 0)
			break;
		thisConf = thisConf->next;
	}
	return thisConf;
}
struct nw_option* find_option(struct nw_config *conf,const char *key)
{
	struct nw_option *thisOpt = NULL;
	if(conf == NULL || key == NULL)
		return NULL;
	thisOpt = conf->options;

	while(thisOpt)
	{
		if(strcmp(thisOpt->key,key) == 0)
			break;
		thisOpt = thisOpt->next;
	}
	return thisOpt;
}
char* find_value(struct nw_config *conf ,const char *key)
{
	struct nw_option *thisOpt = NULL;
	if(conf  == NULL || key == NULL)
		return NULL;
	thisOpt = find_option(conf,key);
	return thisOpt->value;
}
void file_close(struct nw_file **file)
{
	//temp conf pointer
	struct nw_config *thisConf = NULL;
	struct nw_config *nextConf = NULL;

	//temp opt pointer
	struct nw_option *thisOpt = NULL;
	struct nw_option *nextOpt = NULL;

	if(file == NULL || *file == NULL)
		return;
	thisConf = (*file)->configs;
	while(thisConf)
	{
		thisOpt = thisConf->options;
		while(thisOpt)
		{
			nextOpt = thisOpt->next;
			//free option key val
			free(thisOpt->key);
			free(thisOpt->value);
			//free option ptr
			free(thisOpt);
			//move to next
			thisOpt = nextOpt;
		}
		nextConf = thisConf->next;
		//after all options have been freed,free the name of thisConf
		free(thisConf->name);
		free(thisConf);
		thisConf = nextConf;
	}
	free((*file)->name);
	free(*file);
	*file = NULL;
}