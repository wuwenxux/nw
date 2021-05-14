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
 * Add a key value pair option into nw_option object from nw_config;
 * @key ,@value must be valid  if either of them is NULL, return NULL.
 * Return:a nw_option object ,NULL if  operation failed or invalid input(either of config,key,value is NULL).
 * @config:the config interface.
 * @key:key of the option
 * @value:value of the option
 **/
static struct nw_option* add_option(struct nw_config *config,const char *key,const char *value);
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
 * Find the value of a nw_option
 * Return : the value of a nw_option,NULL if not exist
 * @name:name of the config,if NULL return NULL
 * @key:key of the option,if NULL return NULL
 **
char* find_value(struct nw_config *name,const char *key);
*/
/**
 * Find a option in the nw_conf object.
 * @name and @key are supposed to be valid ,if not ,return NULL.
 * Return:a nw_option  if found, else NULL.
 * @name:name of this config
 * @key:key of the option name
 **/
struct nw_option* find_option(struct nw_config *name,const char *key);
static void nw_dev_conf_export(FILE *,
                            const char *,
							const char *,
							const char *,
                            struct nw_other *, 
                            struct nw_bind *,
                            struct nw_ping *,
                            struct nw_type *,
                            struct nw_self *,
							struct nw_peer_entry **,int mptcp_on);
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
		fprintf(stderr,"this file is empty\n");
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
	struct nw_value *thisVal = NULL;
	struct nw_self s;
	struct nw_ping p;
	struct nw_bind b;
	struct nw_type t;
	struct nw_other o;
	struct nw_peer_entry *entry = calloc(1,sizeof(struct nw_peer_entry));	
	memset(&o,0,sizeof(struct nw_other));
	memset(&s,0,sizeof(struct nw_self));
	memset(&b,0,sizeof(struct nw_bind));
	memset(&t,0,sizeof(struct nw_type));
	memset(&p,0,sizeof(struct nw_ping));
	size_t peerIndex;
	int ret;
	char dev[IF_NAMESIZE];
	char ipaddr[16];
	char netmask[16];
	char log[4];
	//char ownid[MAX_PEERNAME_LENGTH];
	bool is_static = false;
	char peer_id[MAX_PEERNAME_LENGTH];
	u32 peer_ip;
	u16 peer_port;
	bool set_mptcp;
	printf("Removing previous configuration....\n");
	sleep(2);
	nw_clear();
	if((file = file_open(path))== NULL)
	{
		fprintf(stderr,"Can't open path %s",path);
		return FILE_NOT_FOUND;
	}
	thisConf = file->configs;
	while(thisConf)
	{
		//printf("set up %s",thisConf->name);
		peerIndex = 0;
		thisOpt = thisConf->options;
		while(thisOpt)
		{	
			//printf("\n%s ",thisOpt->key);
			thisVal = thisOpt->values;
			while(thisVal)
			{
				//printf("%s ",thisVal->string);
				if(strcmp(thisOpt->key,"ifname")==0 )
				{
					strcpy(dev ,thisVal->string);
				}else if(strcmp(thisOpt->key,"proto") == 0)
				{
					is_static = (strcmp(thisVal->string,"dchp") == 0) ? false : true;   
				}
				else if(strcmp(thisOpt->key,"ipaddr") == 0 && is_static)
				{
					strcpy(ipaddr,thisVal->string);
				}else if(strcmp(thisOpt->key,"netmask") == 0)
				{
					strcpy(netmask,thisVal->string);
				}else if(strcmp(thisOpt->key,"mode") == 0)
				{
					if(find_value(thisOpt,"client") == 0)
						t.mode = NW_MODE_CLIENT;
					else if(find_value(thisOpt,"server") == 0)
						t.mode = NW_MODE_SERVER;
				}else if(strcmp(thisOpt->key,"bindport")== 0)
				{
					if(get_unsigned16(&b.port,thisVal->string,10))
					{	
						fprintf(stderr,"Not a valid unsigned short value\n");
					}
				}else if(strcmp(thisOpt->key,"log") == 0)
				{
					if(strcmp(thisVal->string,"yes") == 0 || strcmp(thisVal->string,"no") == 0 )
						strcpy(log,thisVal->string);
					else
					{
						ret = -1;
						goto RETURN;
					}
				}else if(strncmp(thisOpt->key,"peer",4) == 0)
				{
					if(check_opt_peer(dev,thisVal->string,peer_id,&peer_ip,&peer_port))
					{
						ret = -1;
						goto RETURN;
					}
					strcpy(entry->peerid[peerIndex],peer_id);
					entry->ip[peerIndex] = peer_ip;
					entry->port[peerIndex] = peer_port;
					peerIndex++;
				}else if(strcmp(thisOpt->key,"bufflen") == 0)
				{
					assert(thisVal->string != NULL);
					if(get_unsigned32(&o.bufflen,thisVal->string,10))
					{	
						fprintf(stderr,"Not a valid unsigned short value\n.");
						ret = -1;
						goto RETURN;
					}
				}
				else if(strcmp(thisOpt->key,"budget")==0)
				{
					assert(thisVal->string != NULL);
					if(get_unsigned32(&o.budget,thisVal->string,10))
					{	
						fprintf(stderr,"Not a valid unsigned int value\n");
						ret = -1;
						goto RETURN;
					}
				}
				else if(strcmp(thisOpt->key,"oneclient")== 0)
				{
					
					if(find_value(thisOpt,"yes") == 0 ||find_value(thisOpt,"no") == 0)
					{
						strcpy(o.oneclient,thisVal->string);
					} 
					else 
					{
						fprintf(stderr,"Only yes or no.\n");	
						ret = -1;
						goto RETURN;
					}
				}
				else if(strcmp(thisOpt->key,"queuelen") == 0)
				{
					if(get_unsigned32(&o.queuelen,thisVal->string,10))
					{
						fprintf(stderr,"Invalid queuelen val.\n");
						ret = -1;
						goto RETURN;
					}
				}
				else if(strcmp(thisOpt->key,"idletimeout")==0)
				{
					assert(get_unsigned32(&o.idletimeout,thisVal->string,10) == 0);
					if(get_unsigned32(&o.idletimeout,thisVal->string,10))
					{
						fprintf(stderr,"Invalid idletimeout value.\n");
						ret = -1;
						goto RETURN;
					}
				}
				else if(strcmp(thisOpt->key,"batch") == 0)
				{
					assert(get_unsigned32(&o.batch,thisVal->string,10) == 0);
					if(get_unsigned32(&o.batch,thisVal->string,10))
					{
						fprintf(stderr,"Invalid batch value.\n");
						ret = -1;
						goto RETURN;
					}
				}
				else if(strcmp(thisOpt->key,"swtichtimeout")==0)
				{
					assert(get_unsigned32(&o.switchtime,thisVal->string,10)==0);
					if(get_unsigned32(&o.switchtime,thisVal->string,10))
					{
						fprintf(stderr,"Invalid switchtime value\n");
						ret = -1;
						goto RETURN;
					}
				}
				else if(strcmp(thisOpt->key,"interval")==0)
				{
					assert(get_unsigned32(&o.switchtime,thisVal->string,10)==0);
					if(get_unsigned32(&p.interval,thisVal->string,10))
					{
						fprintf(stderr,"Not a valid unsigned int value\n");
					}
				}
				else if(strcmp(thisOpt->key,"timeout")==0)
				{
					assert(get_unsigned32(&p.timeout,thisVal->string,10) == 0);
					if(get_unsigned32(&p.timeout,thisVal->string,10))
					{
						fprintf(stderr,"Not a valid unsigned int value\n");
					}
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
						set_mptcp =false;
					}else 
					{
						on_off("multipath",thisVal->string);
						ret = -1;
						goto RETURN;
					}
				}
				thisVal = thisVal->next;
			}
			thisOpt = thisOpt->next;
		}
		if(!check_ifname(dev)&& !check_ipv4(ipaddr) && !check_netmask(netmask))
	   	{
			ret = nw_setup_dev(dev,ipaddr,netmask);
			if(ret)
			{
				fprintf(stderr,"setup %s failed\n",dev);
				goto RETURN;
			}
			fprintf(stdout,"\ndevice %s setup success\n",dev);
		}
		if(o.batch||o.bufflen||o.idletimeout||o.switchtime||o.budget||o.queuelen||strlen(o.oneclient)||strlen(o.showlog))
		{
			if((ret = nw_other_set(dev,&o)))
				goto RETURN;
		}
		if(p.interval&&p.timeout)
		{	
			if(p.interval < p.timeout)
			{	
				if((ret = nw_ping_set(dev,&p)))
					goto RETURN;
			}
			else
			{
				fprintf(stderr,"Interval comes earlier than timeout.\n");
			}
		}
		if(b.port)
		{
			if((ret =nw_bind_set(dev,&b)))
				goto RETURN;
		}
		if(strlen(s.peerid))
		{
			if((ret =nw_self_set(dev,&s)))
				goto RETURN;
		}
		if(t.mode)
		{
			if((ret =nw_type_set(dev,&t)))
				goto RETURN;
		}
		if(strlen(log)>0)
		{
			strcpy(o.showlog,log);
			if((ret =nw_other_set(dev,&o)))
				goto RETURN;
		}
		if(peerIndex)
		{
			entry->count = peerIndex;
			printf("entry->count:%u\n",entry->count);
			if((ret = nw_do_add(dev,entry)))
				goto RETURN;
		}
		if((ret = nw_mptcp_set(dev,set_mptcp)))
				goto RETURN;
		memset(entry,0,sizeof(struct nw_peer_entry));
		memset(&o,0,sizeof(struct nw_other));
		memset(&s,0,sizeof(struct nw_self));
		memset(&b,0,sizeof(struct nw_bind));
		memset(&t,0,sizeof(struct nw_type));
		memset(&p,0,sizeof(struct nw_ping));
		sleep(1);
		thisConf = thisConf->next;
	}
	goto RETURN;
RETURN:
	free(entry);
	file_close(&file);
	return ret;
}

static int nw_overwrite(char *argv)
{
	char buf[512], ch;

	printf(" overwrite (yes/no)? ");

	memset(buf, 0, sizeof(buf));

	for(;;) {
		ch = fgetc(stdin);
		if (isprint(ch)) {
		//	printf("%c", ch);
			strncat(buf, &ch, 1);
		} else if ((ch == '\n') || (ch == '\r')) {
			printf("%c", ch);
			break;
		} else if (ch == '\b' || ch == 0x7f) {
			/* BackSpace */
			if (strlen(buf)) {
				buf[strlen(buf)-1] = '\0';
				printf("\b");
				printf("\x1b[0J");
			}
		}
	}
	if (buf[0] == 'y') {
		if ((strcmp(buf, "yes") == 0) || strlen(buf) == 1) {
			return 0;
		}
	}
	return -1;
}
int nw_save_conf(char *path)
{
	char *cmd = "ip link show type ngmwan";
	FILE *fp;
	FILE *save_fp;
	int ret;

	//int j= 0;
	char *i,*dev;
	char buf[256];
	if((save_fp = fopen(path,"r")) == NULL)
	{
		//do nothing.
	}else if(nw_overwrite(path) != 0)
	{
		ret = 0;
		goto RETURN;
	}
	memset(buf,0,sizeof(buf));
	if(( fp = popen(cmd,"r")) == NULL)
	{
		perror(strerror(errno));
		ret = -1;
		goto RETURN;
	}
	if((save_fp = fopen(path,"w")) == NULL)
	{
		ret = -1;
		goto RETURN;
	}
	while(fgets(buf,256,fp) != NULL)
	{
		if(*buf != ' ')
		{
			i = strtok(buf,":");
			if(i == NULL)
			{
				ret = -1;
				goto RETURN;
			}
			dev = strtok(NULL, ":");
			if( dev == NULL)
			{
				ret = -1;
				goto RETURN;
			}
			trim(dev,trim_str);
			printf("%s\n",dev);
			if(nw_save_dev(save_fp,dev))
			{
				ret = -1;
				goto RETURN;
			}
		}
	}
	goto RETURN;
RETURN:
	if(fp != NULL)
		pclose(fp);
	if(save_fp != NULL)
		fclose(save_fp);
	return ret;
}
static int nw_save_dev(FILE *fp,const char *dev)
{
	assert(dev != NULL);
	assert(fp != NULL);
	int ret = 0;
	//int i ;
	if (dev  == NULL)
	{
		ret = -1;
		perror("dev is required\n");
		goto RETURN;
	}
	char ip[16];
	char mask[16];
	int on_off = 0;
	//char *cmd = NULL;
	struct nw_other *o = calloc(1,sizeof(struct nw_other));
	struct nw_bind  *b = calloc(1,sizeof(struct nw_bind));
	struct nw_self  *s = calloc(1,sizeof(struct nw_self));
	struct nw_ping  *p = calloc(1,sizeof(struct nw_ping));
	struct nw_type  *t = calloc(1,sizeof(struct nw_type));
	struct nw_peer_entry *npe = calloc(1,sizeof(struct nw_peer_entry));
	if((ret = nw_other_read(dev,o)))
	{
		perror("Nw other info read failure.\n");
		goto RETURN;
	}
	if((ret = nw_ping_read(dev,p)))
	{
		perror("ping info read failure.\n");
		goto RETURN;
	}
	if((ret = nw_type_read(dev,t)))
	{
		perror("type info read failure.\n");
		goto RETURN;
	}
	if((ret = nw_bind_read(dev,b)))
	{
		perror("Nw bind info read failure.\n");
		goto RETURN;
	}
	if(npe == NULL)
	{
		perror("Nw peer entry malloc error.\n");
		goto RETURN;
	}
	if((ret = nw_do_peer_list(dev,npe)))
	{
		perror("Nw peer list info read error.\n");
		goto RETURN;
	}
	if((ret = nw_self_read(dev,s)))
	{
		perror("Nw self info read error.\n");
		goto RETURN;
	}
	//printf("NW info allocating.\n");
	//write
	if((ret = get_ip_mask(dev,ip,mask)))
	{
		perror("Failed to get dev ip and mask.\n");
	}
	on_off = nw_mptcp(dev);
	nw_dev_conf_export(fp,dev,ip,mask,o,b,p,t,s,&npe,on_off);
	ret = 0;
	goto RETURN;
RETURN:
	free(npe);
	free(o);
	free(s);
	free(b);
	free(p);
	free(t);
	return ret;
}
static int nw_remove(const char *dev)
{
	int ret ;
	assert(dev != NULL);
	char rm_cmd[50];
	sprintf(rm_cmd,"sudo ip link del dev %s",dev);
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
	sprintf(show_cmd,"sudo ip link show type ngmwan");
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
	int ret;
	char dev_cmd[50];
	char ip_cmd[128];
	char ip_v4[16];
	char mask_str[16];
	//char if_cmd[128];

	assert(dev != NULL);
	if(check_nw_if(dev) == 0)
	{
		fprintf(stderr,"%s is already exist. change a ifname\n",dev);
		return -1;
	}
	sprintf(dev_cmd,"sudo ip link add %s type ngmwan",dev);
	strcpy(ip_v4,ip_str);
	assert(check_ipv4(ip_v4)==0);
	strcpy(mask_str,netmask);
	assert(check_netmask(mask_str)==0);
	sprintf(ip_cmd,"sudo ip addr add %s/%s dev %s ",ip_v4,mask_str,dev);
	ret = system(dev_cmd);
	if(ret)
	{
		fprintf(stderr,"dev setup failed.\n");
	}
	ret = system(ip_cmd);
	if(ret)
	{
		fprintf(stderr,"ip configuration failed.\n");
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
	if(file == NULL || name == NULL )
	{
		fprintf(stderr,"Neither file nor name can  be NULL.\n");
		exit(EXIT_FAILURE);
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
int  find_value(struct nw_option *opt,const char *val)
{
	struct nw_value *thisVal = NULL;
	if(opt == NULL || val == NULL)
		return -1;
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
		return;
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
							struct nw_peer_entry **npe,int on_off)
{
	int i ;
	char ip_v4[16];
	char ip_mask[16];
	strcpy(ip_v4,dev_ip);
	strcpy(ip_mask,mask);
	assert(s_ptr != NULL);
	assert(npe != NULL);
	assert(dev!=NULL);
	//struct nw_config *thisConfig = NULL;
	//struct nw_config *preConfig = NULL;
	fprintf(fp,"config interface \'%s\'\n",dev);
	fprintf(fp,"\toption ifname \'%s\'\n",dev);
	fprintf(fp,"\toption proto \'static\'\n");
	fprintf(fp,"\toption ipaddr \'%s\'\n",ip_v4);
	fprintf(fp,"\toption netmask \'%s\'\n",ip_mask);
	fprintf(fp,"\toption bufflen \'%u\'\n",o_ptr->bufflen);
	fprintf(fp,"\toption budget \'%u\'\n",o_ptr->budget);
	fprintf(fp,"\toption idletimeout \'%u\'\n",o_ptr->idletimeout);
	fprintf(fp,"\toption oneclient \'%s\'\n",o_ptr->oneclient);
	fprintf(fp,"\toption queuelen \'%u\'\n",o_ptr->queuelen);
	fprintf(fp,"\toption log \'%s\'\n",o_ptr->showlog);
	fprintf(fp,"\toption batch \'%u\'\n",o_ptr->batch);
	fprintf(fp,"\toption switchtime \'%u\'\n",o_ptr->switchtime);
	fprintf(fp,"\toption bindport \'%d\'\n",b_ptr->port);
	fprintf(fp,"\toption interval \'%u\'\n",p_ptr->interval);
	fprintf(fp,"\toption timeout \'%u\'\n",p_ptr->timeout);
	fprintf(fp,"\toption ownid \'%s\'\n",s_ptr->peerid);
	fprintf(fp,"\toption mode \'%s\'\n",t_ptr->mode == NW_MODE_CLIENT ?"client":"server");
	fprintf(fp,"\toption multipath \'%s\'\n",on_off == 0?"off":"on");
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