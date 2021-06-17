#include "nw_cli.h"
#include "nw_err.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "nw_conf.h"
#define trim_str " \'\""

/**
 * Add a configuration of ngmwan and return nw_config object,like:config interface 'nwconf1';
 * @path and @file are supposed to be valid ,return NULL,if either of them is NULL.
 * Return:@nw_config object
 * @file:a nw_file to load in. 
 * @path:path to the file.
 **/
static struct nw_config* add_config(struct nw_file *file,const char *path);
/**
 * Add a nw_option object into  nw_config;
 * @key ,@value must be valid  if either of them is NULL, return NULL.
 * Return:a nw_option object ,NULL if  operation failed or invalid input(either of config,key,value is NULL).
 * @config:the config interface.
 * @key:key of the option
 * @value:value of the option
 **/
static struct nw_option* add_option(struct nw_config *config,const char *key,const char *value);
/**
 * Add a value to nw_val object.
 * @opt,value must be valid if either of them is NULL, return NULL.
 * Return: a nw_option object, NULL if not exist,or param error.
 * @nw_option:nw_option object.
 * @opt:the option object.
 * @value:val of this opt.
 **/
static struct nw_option* add_value(struct nw_option *opt, const char *value);
/**
 * Find a nw_config from the nw_file file
 * @file and @name should be valid input, if NULL or invalid return NULL. 
 * Return:A nw_config listhead object.
 * @file:a nw_file to load 
 * @name:name of this nw_config
 **/ 
struct nw_config* find_config(struct nw_file *file,const char *name);
/**
 * Find a option in the nw_conf object.
 * @name and @key are supposed to be valid ,if not ,return NULL.
 * Return:a nw_option  if found, else NULL.
 * @name:name of this config
 * @key:key of the option name
 **/
struct nw_option* find_option(struct nw_config *name,const char *key);
static int nw_load_dev(struct nw_config *);
static void nw_dev_conf_export(FILE *,
                            const char *,
							const char *,
							const char *,
                            struct nw_other *, 
                            struct nw_bind *,
                            struct nw_ping *,
                            struct nw_type *,
                            struct nw_self *,
							struct nw_dhcp *,
							struct nw_peer_entry **,int mptcp_on);
struct nw_file *file_open(char *path)
{
	char *word = NULL;
	char *conf_name = NULL; 
	char *inface = NULL;
	char *key,*val;
	char  line[BUFLEN];
	FILE *confFile = NULL;
	struct nw_file *file = NULL;
	struct nw_config *thisConfig = NULL;

	if((confFile = fopen(path,"r"))== NULL)
	{
		fprintf(stderr,"Error:This file is not exist.\n");
		return NULL;
	}
	//file malloc 
	file = malloc(sizeof(struct nw_file));
	if(file == NULL)
	{
		goto malloc_err;
	}
	file->name = malloc(strlen(path)+1);
	if(file->name == NULL)
	{
		goto malloc_err;
	}
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
			 	trim(conf_name,trim_str);
				thisConfig = add_config(file,conf_name);
		   }else
		   {
			   	fprintf(stderr,"Interface is expected.\n");
		   		goto exit;
		   }
		}else if(strcmp(word,"option")== 0)
		{
			key = strtok(NULL," ");
		    val = strtok(NULL," ");
		   	assert(key != NULL);
			assert(val != NULL);
			if(key == NULL || val == NULL)
			{
				goto params_error;
			}
			trim(val,trim_str);
		 	add_option(thisConfig,key,val);
		}
	}
		goto exit;
malloc_err:
	fprintf(stderr,"Error:file malloc failed.\n");
	fclose(confFile);
	return NULL;
exit:
    fclose(confFile);
	return file;
params_error:
	fprintf(stderr,"option key val is expected.\n");
	return NULL;
}
int nw_reload_conf(char *path)
{
	struct nw_file *file = NULL;
	struct nw_config *thisConf = NULL;
	struct nw_option *thisOpt = NULL;
	struct nw_value *thisVal = NULL;
	char *dev = NULL;
	int ret = -1;
	if((file = file_open(path)) == NULL)
	{
		goto file_not_exist;
	}
	for(thisConf = file->configs; thisConf; thisConf = thisConf->next)
	{
		thisOpt = find_option(thisConf,"ifname");
		assert( thisOpt->values != NULL);
		thisVal = thisOpt->values;
		dev = thisVal->string;
		if((ret = check_nw_if(dev)) == 0)
		{
			printf("Ignore %s (already exist)...\n",dev);
			sleep(1);
			continue;
		}else if( ret == DEV_NOT_FOUND)
		{
			printf("Reloading %s...\n",dev);
			ret = nw_load_dev(thisConf);
			if(ret)
				goto fail;
		}else
		{
			//do nothing,unexpected.
			goto unexpected_err;
		}
	}
	goto success;
file_not_exist:
	fprintf(stderr,"Error:file not exist.\n");
	return -1;
unexpected_err:
	fprintf(stderr,"Error:unexpected error.\n");
	file_close(&file);
	return -1;
fail:
	file_close(&file);
	return -1;
success:
	file_close(&file);
	return 0;
}
static int nw_load_dev(struct nw_config *config)
{
	struct nw_self s;
	struct nw_ping p;
	struct nw_bind b;
	struct nw_type t;
	struct nw_other o;
	struct nw_dhcp d;
	struct nw_peer_entry *entry = calloc(1,sizeof(struct nw_peer_entry));
	memset(&o,0,sizeof(struct nw_other));
	memset(&s,0,sizeof(struct nw_self));
	memset(&b,0,sizeof(struct nw_bind));
	memset(&t,0,sizeof(struct nw_type));
	memset(&p,0,sizeof(struct nw_ping));
	memset(&d,0,sizeof(struct nw_ping));
	assert(config != NULL);
	if(config == NULL)
		return -1;
	size_t peerIndex=0;
	int ret;
	char dev[IF_NAMESIZE];
	char ipaddr[16];
	char netmask[16];
	bool ping_set[2]= {false};
	bool is_static = false;
	char peer_id[MAX_PEERNAME_LENGTH];
	u32 peer_ip;
	u16 peer_port;
	bool set_mptcp = false;
	struct nw_config *thisConf = NULL;
	struct nw_option *thisOpt = NULL;
	struct nw_value *thisVal = NULL;
	thisConf = config;
	thisOpt = thisConf->options;
	thisOpt = thisConf->options;

	while(thisOpt)
	{	
		thisVal = thisOpt->values;
		while(thisVal)
		{
			if(strcmp(thisOpt->key,"ifname") == 0 )
			{
				strcpy(dev ,thisVal->string);
			}else if(strcmp(thisOpt->key,"ipaddr") == 0)
			{
				strcpy(ipaddr,thisVal->string);
				is_static = true;
			}else if(strcmp(thisOpt->key,"netmask") == 0)
			{
				strcpy(netmask,thisVal->string);
			}else if(strcmp(thisOpt->key,"mode") == 0)
			{
				if(find_value(thisOpt,"client") == 0)
					t.mode = NW_MODE_CLIENT;
				else if(find_value(thisOpt,"server") == 0)
					t.mode = NW_MODE_SERVER;
				else
				{
					ret = cli_ser(thisOpt->key,thisVal->string);
					goto result;
				}
			}else if(strcmp(thisOpt->key,"bindport")== 0)
			{
				if(get_unsigned16(&b.port,thisVal->string,10))
				{	
					fprintf(stderr,"Error:\'%s\' param \'%s\' : \'%s\' is not a valid unsigned short value\n",thisConf->name,thisOpt->key,thisVal->string);
					goto params_err;
				}
			}else if(strcmp(thisOpt->key,"log") == 0)
			{
				if(strcmp(thisVal->string,"yes") == 0 || strcmp(thisVal->string,"no") == 0 )
					strcpy(o.showlog,thisVal->string);
				else
				{
					ret = yes_no(thisOpt->key,thisVal->string);
					goto result;
				}
			}else if(strcmp(thisOpt->key,"autopeer") == 0)
			{
				if(strcmp(thisVal->string,"yes") == 0 || strcmp(thisVal->string,"no") == 0)
				{
					strcpy(o.autopeer,thisVal->string);
				}else
				{
					ret = yes_no(thisOpt->key,thisVal->string);
					goto result;
				}
			}else if(strcmp(thisOpt->key,"compress") == 0)
			{
				if(strcmp(thisVal->string,"yes") == 0 || strcmp(thisVal->string,"no") == 0)
				{
					strcpy(o.compress,thisVal->string);
				}else
				{
					ret = yes_no(thisOpt->key,thisVal->string);
					goto result;
				}
			}else if(strcmp(thisOpt->key,"simpleroute") == 0)
			{
				if(strcmp(thisVal->string,"yes") == 0 || strcmp(thisVal->string,"no") == 0)
				{
					strcpy(o.simpleroute,thisVal->string);
				}else
				{
					ret = yes_no(thisOpt->key,thisVal->string);
					goto result;
				}
			}else if (strcmp(thisOpt->key,"isolate") == 0)
			{
				if(strcmp(thisVal->string,"yes") == 0 || strcmp(thisVal->string,"no") == 0)
				{
					strcpy(o.isolate,thisVal->string);
				}else
				{
					ret = yes_no(thisOpt->key,thisVal->string);
					goto result;
				}
			}else if(strncmp(thisOpt->key,"peer",4) == 0)
			{
				if(check_opt_peer(dev,thisVal->string,peer_id,&peer_ip,&peer_port))
				{
					fprintf(stderr,"Error:\'%s\' param \'%s\' :invalid peerid string %s\n",thisConf->name,thisOpt->key,thisVal->string);
					ret = -1;
					goto result; 
				}
				strcpy(entry->peerid[peerIndex],peer_id);
				entry->ip[peerIndex] = peer_ip;
				entry->port[peerIndex] = peer_port;
				peerIndex++;
			}else if(strcmp(thisOpt->key,"budget") == 0)
			{
				assert(thisVal->string != NULL);
				if(get_unsigned32(&o.budget,thisVal->string,10))
				{	
					fprintf(stderr,"Error:\'%s\' param \'%s\':can be divided by 64.\n",thisConf->name ,thisOpt->key);
					ret = -1;
					goto result;
				}
			}else if(strcmp(thisOpt->key,"oneclient")== 0)
			{
				if(find_value(thisOpt,"yes") == 0 ||find_value(thisOpt,"no") == 0)
				{
					strcpy(o.oneclient,thisVal->string);
				} 
				else 
				{
					fprintf(stderr,"Error:\'%s\' param \'%s\':should be only yes or no.\n",thisConf->name ,thisOpt->key);	
					ret = -1;
					goto result;
				}
			}else if(strcmp(thisOpt->key,"queuelen") == 0)
			{
				if(get_unsigned32(&o.queuelen,thisVal->string,10))
				{
					fprintf(stderr,"Error:\'%s\' param \'%s\' is invalid.\n",thisConf->name,thisOpt->key);
					ret = -1;
					goto result;
				}
			}else if(strcmp(thisOpt->key,"idletimeout")==0)
			{
				if(get_unsigned32(&o.idletimeout,thisVal->string,10))
				{
					fprintf(stderr,"Error:\'%s\' param \'%s\':invalid idletimeout value.\n",thisConf->name,thisOpt->key);
					ret = -1;
					goto result;
				}
			}else if(strcmp(thisOpt->key,"swtichtimeout") ==0)
			{
				assert(get_unsigned32(&o.switchtime,thisVal->string,10)==0);
				if(get_unsigned32(&o.switchtime,thisVal->string,10))
				{
					fprintf(stderr,"Error:\'%s\' param \'%s\':invalid switchtime value\n",thisConf->name,thisOpt->key);
					ret = -1;
					goto result;
				}
			}
			else if(strcmp(thisOpt->key,"interval")==0)
			{
				assert(get_unsigned32(&o.switchtime,thisVal->string,10)==0);
				if(get_unsigned32(&p.interval,thisVal->string,10))
				{
					fprintf(stderr,"Error:\'%s\' param \'%s\':not a valid unsigned int value\n",thisConf->name,thisOpt->key);
					ret = -1;
					goto result;
				}
				ping_set[0]= true;
			}
			else if(strcmp(thisOpt->key,"timeout")==0)
			{
				assert(get_unsigned32(&p.timeout,thisVal->string,10) == 0);
				if(get_unsigned32(&p.timeout,thisVal->string,10))
				{
					fprintf(stderr,"Error:\'%s\' param \'%s\':not a valid unsigned int value\n",thisConf->name,thisOpt->key);
					ret = -1;
					goto result;
				}
				ping_set[1] = true;
			}
			else if(strcmp(thisOpt->key,"ownid")==0 && check_self(thisVal->string)== 0)
			{	
				strcpy(s.peerid,thisVal->string);
			}else if(strcmp(thisOpt->key,"multipath") == 0 || strcmp(thisOpt->key,"mptcp") == 0)
			{
				if(strcmp(thisVal->string,"on") == 0)
				{
					set_mptcp = true;
				}
				else if(strcmp(thisVal->string,"off") == 0)
				{ 
					set_mptcp = false;
				}else 
				{
					ret = on_off("multipath",thisVal->string);
					goto result;
				}
			}else if(strcmp(thisOpt->key,"dhcp") == 0)
			{
				assert(thisVal->string != NULL);
				if(strcmp(thisVal->string,"yes") == 0)
				{
					strcpy(d.enable,"yes");
					is_static = false;
				}else if(strcmp(thisVal->string,"no") == 0)
				{
					is_static = true;
					strcpy(d.enable,"no");
				}else
				{
					ret = yes_no("dhcp",thisVal->string);
					goto result;
				}
			}else if(strcmp(thisOpt->key,"dhcp-startip") == 0)
			{
				assert(thisVal->string != NULL);
				if((ret = check_ipv4(thisVal->string)))
				{
					fprintf(stderr,"Error:%s is not a valid dhcp-ip addr.",thisVal->string);
					goto result;
				}
				ret = inet_pton(AF_INET,thisVal->string,&d.startip);
				if(ret < 1)
				{
					fprintf(stderr,"Error:%s convert to be32 failure.\n",thisVal->string);
					goto result;
				}
			}else if (strcmp(thisOpt->key,"dhcp-endip") == 0)
			{
				assert(thisVal->string != NULL);
				if((ret = check_ipv4(thisVal->string)))
				{
					fprintf(stderr,"Error:%s is not a valid dhcp-ip addr.",thisVal->string);
					goto result;
				}
				ret = inet_pton(AF_INET,thisVal->string,&d.endip);
				if(ret < 1)
				{
					fprintf(stderr,"Error:%s convert to u32 failure.\n",thisVal->string);
					goto result;
				}
			}else if(strcmp(thisOpt->key,"dhcp-mask") == 0)
			{
				assert(thisVal->string != NULL);
				if((ret = check_netmask(thisVal->string)))
				{
					fprintf(stderr,"Error: %s:%s is not valid.\n",thisOpt->key,thisVal->string);
					goto result;
				}
				ret = inet_pton(AF_INET,thisVal->string,&d.mask);
				if(ret < 1)
				{
					fprintf(stderr,"Error: %s convert to u32 failure.\n",thisVal->string);
					goto result;
				}
			}
				thisVal = thisVal->next;
			}
			thisOpt = thisOpt->next;
		}
		if(check_ifname(dev))
	   	{
			ret = -1;
			fprintf(stderr,"Error:%s is invalid.\n",dev);
			goto result;
		}
		if(is_static)
		{
			if( check_ipv4(ipaddr) || check_netmask(netmask))
			{
				ret = -1;
				fprintf(stderr,"Error:dev %s: %s or %s is wrong, param error.\n",dev,ipaddr,netmask);
				goto result;
			}
		}
		if((ret = nw_setup_dev(dev,ipaddr,netmask)))
		{
			fprintf(stderr,"Error:setup %s failed\n",dev);
			goto result;
		}
		if(strlen(o.simpleroute)||strlen(o.compress)||
			o.switchtime||o.idletimeout||
			strlen(o.isolate)||strlen(o.showlog)||
			strlen(o.oneclient)||o.queuelen||
			o.budget||strlen(o.autopeer))
		{
			if((ret = nw_other_set(dev,&o)))
				goto result;
		}
		if(ping_set[0] == true && ping_set[1] == true)
		{	
			if(p.interval < p.timeout)
			{	
				if((ret = nw_ping_set(dev,&p)))
					goto result;
			}else
			{
				fprintf(stderr,"Error:%s interval shoule be smaller than timeout.\n",dev);
				goto params_ping_err;
			}
		}
		if(b.port)
		{
			if((ret = nw_bind_set(dev,&b)))
				goto result;
		}
		if(strlen(s.peerid))
		{
			if((ret = nw_self_set(dev,&s)))
				goto result;
		}
		if(t.mode)
		{
			if((ret = nw_type_set(dev,&t)))
				goto result;
		}
		if(strcmp(d.enable,"yes") == 0 )
		{
			if((ret = nw_dhcp_set(dev,&d)))
				goto result;
		}
		if(peerIndex)
		{
			entry->count = peerIndex;
			if((ret = nw_do_add(dev,entry)))
				goto result;
		}
		if((ret = nw_mptcp_set(dev,set_mptcp)))
				goto result;
		ret = nw_dev_on(dev);
		if(ret)
		{
			fprintf(stderr,"Error:%s start up failed.\n",dev);
			return ret;
		}
		fprintf(stdout,"Dev %s set up success.\n",dev);
		memset(entry,0,sizeof(struct nw_peer_entry));
		memset(&o,0,sizeof(struct nw_other));
		memset(&s,0,sizeof(struct nw_self));
		memset(&b,0,sizeof(struct nw_bind));
		memset(&t,0,sizeof(struct nw_type));
		memset(&p,0,sizeof(struct nw_ping));
		memset(&d,0,sizeof(struct nw_dhcp));
		ping_set[0] = false;
		ping_set[1] = false;
		set_mptcp = false;
		peerIndex = 0;
		return 0;
params_err:
params_ping_err:
	free(entry);
	return -1;
result:
	free(entry);
	return ret;
}
int nw_load_conf(char *path)
{
	struct nw_file *file  = NULL;
	struct nw_config *thisConf = NULL;
	int ret = -1;
	printf("Processing ...\n");
	nw_clear();
	sleep(1);
	if((file = file_open(path))== NULL)
	{
		fprintf(stderr,"Can't open path %s\n",path);
		return FILE_NOT_FOUND;
	}
	thisConf = file->configs;
	while(thisConf)
	{
		ret = nw_load_dev(thisConf);
		thisConf = thisConf->next;
	}
	goto result;
result:
	file_close(&file);
	return ret;
}
int nw_save_conf(char *path)
{
	char *cmd = "ip link show type ngmwan";
	FILE *fp = NULL;
	FILE *save_fp = NULL;
	int ret;

	char *i,*dev;
	char buf[256];
	memset(buf,0,sizeof(buf));
	if(( fp = popen(cmd,"r")) == NULL)
	{
		perror(strerror(errno));
		ret = -1;
		goto popen_err;
	}
	if((save_fp = fopen(path,"w")) == NULL)
	{
		fprintf(stderr,"Can't open file: %s.\n",path);
		ret = -1;
		goto result;
	}
	printf("%s\n",path);
	while(fgets(buf,256,fp) != NULL)
	{
		if(*buf != ' ')
		{
			i = strtok(buf,":");
			if(i == NULL)
			{
				ret = -1;
				goto result;
			}
			dev = strtok(NULL, ":");
			if( dev == NULL)
			{
				ret = -1;
				goto result;
			}
			trim(dev,trim_str);
			printf("%s\n",dev);
			if(nw_save_dev(save_fp,dev))
			{
				ret = -1;
				fprintf(stderr,"Conf %s save failed.\n",dev);
				goto result;
			}
			sleep(1);
			printf("Config %s save success.\n",dev);
		}
	}
	goto result;
popen_err:
	if(fp != NULL)
		pclose(fp);

result:
	if(save_fp != NULL)
		fclose(save_fp);
	if(fp != NULL)
		pclose(fp);
	return ret;
}
static int nw_save_dev(FILE *fp,const char *dev)
{
	assert(dev != NULL);
	assert(fp != NULL);
	int ret = 0;
	if (dev  == NULL || fp == NULL)
	{
		ret = -1;
		perror("dev or fp can not be null.\n");
		goto result;
	}
	char ip[16];
	char mask[16];
	int on_off = 0;
	//char *cmd = NULL;
	struct nw_other o;
	struct nw_bind  b;
	struct nw_self  s;
	struct nw_ping  p;
	struct nw_type  t;
	struct nw_dhcp  d;
	struct nw_peer_entry *npe = calloc(1,sizeof(struct nw_peer_entry));
	assert(npe!= NULL);
	if((ret = nw_other_read(dev,&o)))
	{
		perror("Nw other info read failure.\n");
		goto result;
	}
	if((ret = nw_ping_read(dev,&p)))
	{
		perror("Nw ping info read failure.\n");
		goto result;
	}
	if((ret = nw_type_read(dev,&t)))
	{
		perror("Nw type info read failure.\n");
		goto result;
	}
	if((ret = nw_bind_read(dev,&b)))
	{
		perror("Nw bind info read failure.\n");
		goto result;
	}
	if((ret = nw_dhcp_read(dev,&d)))
	{
		perror("Nw dhcp info read failure.\n");
		goto result;
	}
	if((ret = nw_do_peer_list(dev,npe)))
	{
		perror("Nw peer list info read error.\n");
		goto result;
	}
	if((ret = nw_self_read(dev,&s)))
	{
		perror("Nw self info read error.\n");
		goto result;
	}
	//write
	if((ret = get_ip_mask(dev,ip,mask)))
	{
		perror("Failed to get dev ip and mask.\n");
		goto result;
	}
	on_off = nw_mptcp(dev);
	nw_dev_conf_export(fp,dev,ip,mask,&o,&b,&p,&t,&s,&d,&npe,on_off);
	ret = 0;
	goto result;
result:
	free(npe);
	return ret;
}
static int nw_remove(const char *dev)
{
	int ret ;
	assert(dev != NULL);
	char rm_cmd[50];
	sprintf(rm_cmd,"ip link del dev %s",dev);
	ret = system(rm_cmd);
	if(ret)
		return -1;
	return 0;
}
static void nw_clear()
{
	char show_cmd[50];
	FILE *fp = NULL;
	char buf[256];
	memset(buf,0,256);
	sprintf(show_cmd,"ip link show type ngmwan");
	char *i,
		 *d;
	if((fp = popen(show_cmd,"r")) == NULL)
	{
		perror(strerror(errno));
		return ;
	}
	while(fgets(buf,256,fp) != NULL)
	{
		if(*buf != ' ')
		{
			i = strtok(buf,":");
			if(i == NULL)
			{
				pclose(fp);
				return ;
			}
			d = strtok(NULL,":");
			if(d == NULL)
			{
				pclose(fp);
				return ;
			}
			trim(d,trim_str);
			if(nw_remove(d))
			{
				pclose(fp);
				return ;
			}
		}
	}
	pclose(fp);
	return ;
}
int nw_setup_dev(const char *dev, char *ip_str, char *netmask)
{
	int ret = -1;
	char dev_cmd[50];
	char dev_down[50];
	char set_tx[50];
	char ip_cmd[128];
	char ip_v4[16];
	char mask_str[16];

	assert(dev != NULL);
	if(check_nw_if(dev) == 0)
	{
		fprintf(stderr,"%s is already exist, change a ifname.\n",dev);
		return -1;
	}
	sprintf(dev_cmd,"ip link add %s type ngmwan",dev);
	sprintf(dev_down,"ip link set %s down",dev);
	sprintf(set_tx,"ip link set %s txqueuelen 1000",dev);
	strcpy(ip_v4,ip_str);
	//printf("%s",ip_v4);
	//assert(check_ipv4( ip_v4 ) == 0);
	strcpy(mask_str,netmask);
	assert(check_netmask( mask_str )==0);
	ret = system(dev_cmd);
	if(ret)
	{
		fprintf(stderr,"Error:dev setup failed.\n");
		return -1;
	}
	ret = system(dev_down);
	if(ret)
	{
		fprintf(stderr,"Error:dev set down failed.\n");
		return -1;
	}
	ret = system(set_tx);
	if(ret)
	{
		fprintf(stderr,"Error:dev set txqueuelen failed.\n");
		return -1;
	}
	if(strlen(ip_str))
	{
		sprintf(ip_cmd,"ip addr add %s/%s dev %s ",ip_v4,mask_str,dev);
		ret = system(ip_cmd);
		if(ret)
		{
			fprintf(stderr,"Error:ip configuration failed.\n");
		}
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
		fprintf(stderr,"Neither file nor name can be NULL.\n");
		exit(EXIT_FAILURE);
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
static struct nw_option* add_value(struct nw_option *thisOpt ,const char *value)
{
	//allocate  a new value item 
	struct nw_value *thisVal = NULL;
	struct nw_value *prevVal = NULL;
	thisVal = malloc(sizeof(struct nw_value));

	if(thisVal == NULL)
	{
		perror("struct value malloc error.\n");
		exit(EXIT_FAILURE);
	}
	thisVal->string = malloc(strlen(value)+1);
	
	if(thisVal->string == NULL)
	{
		perror("char * value malloc error.\n");
		free(thisOpt->values);
		exit(EXIT_FAILURE);
	}
	memcpy(thisVal->string,value,strlen(value) + 1);
	thisVal->next = NULL;
	//first val
	prevVal = thisOpt->values;
	if(prevVal == NULL)
	{
		thisOpt->values = thisVal;
		return thisOpt;
	}
	while(prevVal && prevVal->next)
	{
		prevVal = prevVal->next;
	}
	prevVal->next = thisVal;
	return thisOpt;
}
static struct nw_option* add_option(struct nw_config *conf,const char *key,const char *val)
{
	struct nw_option *thisOpt = NULL;
	struct nw_option *prevOpt = NULL;
	assert(key != NULL);
	assert(val != NULL);
	assert(conf != NULL);
	if((thisOpt = find_option(conf,key)) != NULL)
	{
		assert(val != NULL);
		add_value(thisOpt,val);
		return thisOpt;
	}
	thisOpt = malloc(sizeof(struct nw_option));
	//key assignment 
	thisOpt->key = malloc(strlen(key)+1);
	memcpy(thisOpt->key,key,strlen(key)+1);
	
	add_value(thisOpt,val);
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
	assert(file != NULL);
	if(file == NULL || name == NULL )
	{
		fprintf(stderr,"File not exist,param error.\n");
		return NULL;
	}
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
	{
		fprintf(stderr,"Conf not exist,param error.\n");
		return NULL;
	}
	thisOpt = conf->options;

	while(thisOpt)
	{
		if(strcmp(thisOpt->key,key) == 0)
			break;
		thisOpt = thisOpt->next;
	}
	return thisOpt;
}
int  find_value(struct nw_option *opt,const char *val)
{
	struct nw_value *thisVal = NULL;
	if(opt == NULL || val == NULL)
	{
		fprintf(stderr,"Opt not exist,param error.\n");
		return -1;
	}
	thisVal = opt->values;
	while(thisVal)
	{
		if(strcmp(thisVal->string,val) == 0)
			return 0;
		thisVal = thisVal->next;
	}
	return -1;
}
void file_close(struct nw_file **file)
{
	//temp conf pointer
	struct nw_config *thisConf = NULL;
	struct nw_config *nextConf = NULL;

	//temp opt pointer
	struct nw_option *thisOpt = NULL;
	struct nw_option *nextOpt = NULL;

	//temp val pointer
	struct nw_value *thisVal = NULL;
	struct nw_value *nextVal = NULL;

	assert(file != NULL || *file != NULL);
	if(file == NULL || *file == NULL)
	{
		fprintf(stderr,"File not exist, params error.\n");
		return;
	}

	thisConf = (*file)->configs;
	while(thisConf)
	{
		nextConf = thisConf->next;
		thisOpt = thisConf->options;
		while(thisOpt)
		{
			nextOpt = thisOpt->next;

			thisVal = thisOpt->values;
			while(thisVal)
			{
				nextVal = thisVal->next;
				free(thisVal->string);
				free(thisVal);
				thisVal = nextVal;
			}		
			//free option key val
			free(thisOpt->key);
			//free option ptr
			free(thisOpt);
			//move to next
			thisOpt = nextOpt;
		}
		//after all options have been freed,free the name of thisConf
		free(thisConf->name);
		free(thisConf);
		thisConf = nextConf;
	}
	free((*file)->name);
	free(*file);
	*file = NULL;

}
static void nw_dev_conf_export(FILE *fp,
                            const char *dev,
							const char *dev_ip,
							const char *mask,
                            struct nw_other *o_ptr, 
                            struct nw_bind *b_ptr,
                            struct nw_ping *p_ptr,
                            struct nw_type *t_ptr,
                            struct nw_self *s_ptr,
							struct nw_dhcp *d_ptr,
							struct nw_peer_entry **npe,int on_off)
{
	int i ;
	char d_ip_s[16];
	char d_ip_e[16];
	char d_ip_m[16];
	char ip_v4[16];
	char ip_mask[16];
	strcpy(ip_v4,dev_ip);
	strcpy(ip_mask,mask);
	
	assert(s_ptr != NULL);
	assert(npe != NULL);
	assert(dev != NULL);

	fprintf(fp,"config interface \'%s\'\n",dev);
	fprintf(fp,"\toption ifname \'%s\'\n",dev);
	fprintf(fp,"\toption ipaddr \'%s\'\n",ip_v4);
	fprintf(fp,"\toption netmask \'%s\'\n",ip_mask);
	fprintf(fp,"\toption budget \'%u\'\n",o_ptr->budget);
	fprintf(fp,"\toption idletimeout \'%u\'\n",o_ptr->idletimeout);
	fprintf(fp,"\toption oneclient \'%s\'\n",o_ptr->oneclient);
	fprintf(fp,"\toption autopeer \'%s\'\n",o_ptr->autopeer);
	fprintf(fp,"\toption compress \'%s\'\n",o_ptr->compress);
	fprintf(fp,"\toption simpleroute \'%s\'\n",o_ptr->simpleroute);
	fprintf(fp,"\toption queuelen \'%u\'\n",o_ptr->queuelen);
	fprintf(fp,"\toption log \'%s\'\n",o_ptr->showlog);
	fprintf(fp,"\toption isolate \'%s\'\n",o_ptr->isolate);
	fprintf(fp,"\toption switchtime \'%u\'\n",o_ptr->switchtime);
	fprintf(fp,"\toption bindport \'%d\'\n",b_ptr->port);
	fprintf(fp,"\toption interval \'%u\'\n",p_ptr->interval);
	fprintf(fp,"\toption timeout \'%u\'\n",p_ptr->timeout);
	fprintf(fp,"\toption ownid \'%s\'\n",s_ptr->peerid);
	fprintf(fp,"\toption mode \'%s\'\n",t_ptr->mode == NW_MODE_CLIENT ?"client":"server");
	fprintf(fp,"\toption multipath \'%s\'\n",on_off == 0?"off":"on");
	fprintf(fp,"\toption dhcp \'%s\'\n",d_ptr->enable);

	if(t_ptr->mode == NW_MODE_SERVER)
	{
		if(inet_ntop(AF_INET,&d_ptr->startip,d_ip_s,16) == NULL)
		{
			perror("inet_ntop dhcp_start iperror.\n");
		}
		fprintf(fp,"\toption dhcp-startip \'%s\'\n",d_ip_s);

		if(inet_ntop(AF_INET,&d_ptr->endip,d_ip_e,16) == NULL)
		{
			perror("inet_ntop dhcp_end error.\n");
		}
		fprintf(fp,"\toption dhcp-endip \'%s\'\n",d_ip_e);
		
		if(inet_ntop(AF_INET,&d_ptr->mask,d_ip_m,16) == NULL)
		{
			perror("inet_ntop dhcp_mask error.\n");
		}
		fprintf(fp,"\toption mask \'%s\'\n",d_ip_m);
	}
	for(i = 0; i < (*npe)->count; i++)
	{
		if(inet_ntop(AF_INET,&((*npe)->ip[i]),ip_v4,16) == NULL)
		{
			perror("inet_ntop error.\n");
		}
		fprintf(fp,"\toption peer \'%s,%s,%d\'\n",(*npe)->peerid[i],ip_v4,(*npe)->port[i]);
	}
	fprintf(fp,"\n");
	return;
}