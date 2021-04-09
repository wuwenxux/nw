#include "nw_cli.h"
#include "nw_err.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include "nw_conf.h"
#define trim_str "\'\""

static struct nw_config* add_config(struct nw_file *,const char *);
static struct nw_option* add_option(struct nw_config *,const char *,const char *);
struct nw_config* find_config(struct nw_file *,const char *);
char* find_value(struct nw_config *,const char *);
struct nw_option* find_option(struct nw_config *,const char *);

struct nw_file *file_open(char *path)
{
	char *word;
	char *conf_name;
	char *inface;
	char *key,*val;
	char line[BUFLEN];
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
int nw_save_conf(int argc, char **argv)
{
	//struct nw_other *other;
	//nw_other_save(other);
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