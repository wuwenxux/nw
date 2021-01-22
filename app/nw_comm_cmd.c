#include "nw_cli.h"
#include "utils.h"
#include <err.h>
int nw_search_if( char *dev)
{
	FILE *fp;
	char buf[512];
	char *cmdline = "/sbin/ip link";
	char *i,*n;
	if((fp = popen(cmdline,"r")) == NULL)
	{
		err(EXIT_FAILURE,"%s",cmdline);
		return -1;
	}
	
	memset(buf,0,sizeof(buf));

	/*nw%d device search*/
	while(fgets(buf,512,fp) != NULL){
		if(*buf != ' ')
		{
			i = strtok(buf,":");
			if(i == NULL)
			{
				printf("cmd error\n");
				pclose(fp);
				return -1;
			}
			n = strtok(NULL,":");
			if(n == NULL)
			{
				printf("cmd error\n");
				pclose(fp);
			}
			if(strncmp(&n[1],"nw",2) == 0)
			{
				strcpy(dev,&n[1]);
				return 0;
			}
		}
	}
	pclose(fp);
	
	/* not exist net device */
	return -1;
}
static int yes_no(const char *msg, const char *realval)
{
	fprintf(stderr,
		"Error: argument of \"%s\" must be \"yes\" or \"no\", not \"%s\"\n",
		msg, realval);
	return -1;
}
int nw_dev_show(int argc, char **argv)
{
    int ret;
    
    return 0;
}
int nw_dev_bindport(int argc, char **argv)
{

    return 0;
}

int nw_self_ownid(const char *dev, char *ownid)
{
	return 0;
}
int nw_dev_set(int argc, char **argv)
{
	struct nw_other other;
	memset(&other,0,sizeof(struct nw_other));
	other.bufflen = 1024;
	other.maxbufflen = 0;
	other.queuelen = 16384;
	nw_mode_t mode ;
	strncpy(&other.oneclient, "yes",3);			//参数值，yes/no
	strncpy(&other.showlog , "no",2); ;			//
	other.batch = 80;
	other.idletimeout= 0;			//单位秒
	while(argc > 0)
	{
		if(matches(*argv,"bufflen") == 0)
		{
			NEXT_ARG();
			if( bufflen != 1024)
				duparg("bufflen",*argv);
			if(get_unsigned(&other.bufflen,*argv,0) || bufflen > 102400 || bufflen < 64)
				invarg("Invalid \"bufflen\" value\n",*argv);
			maxbufflen = (maxbufflen < bufflen )? bufflen : maxbufflen;
		}else if(matches(*argv,"maxbufflen") == 0 || matches(*argv,"maxqlen") == 0)
		{
			NEXT_ARG();
			if(other.maxbufflen != 0 )
				duparg("maxbufflen",*argv);
			if(get_unsigned(&other.maxbufflen,*argv,0) || other.maxbufflen < bufflen )
				invarg("Invalid \"maxbufflen\" value\n",*argv);
		}else if(matches(*argv,"queuelen") || matches(*argv,"qlen") == 0)
		{
			NEXT_ARG();
			if(other.queuelen != 16384)
				duparg("queuelen",*argv);
			if(get_unsigned(&other.queuelen,*argv,0) || other.queuelen > 1000000 || other.queuelen < 1000)
				invarg("Invalid \"maxbufflen\" value\n",*argv);
		}else if(matches(*argv,"oneclient") || matches(*argv,"onecli") == 0)
		{
			NEXT_ARG();
			if(strcmp(*argv,"yes") == 0 )
			{
				strncpy(&other.oneclient,"yes",3);
			}else if(strcmp (*argv, "no"))
			{
				strncpy(&other.oneclient,"no",2);
			}else
			{
				return yes_no("oneclient",*argv);
			}
			
		}
		else if (matches(*argv,"log") == 0)
		{
			NEXT_ARG();
			if(strcmp(*argv,"yes") == 0)
			{
				strcpy(&other.showlog,"yes");
			}else(strcmp(*argv,"no") == 0)
			{
				strcpy(&other.showlog,"no");
			}else
			{
				return yes_no("log",*argv);
			}
			
		}else if (matches(*argv,"batch") == 0 || matches(*argv,"bat") == 0)
		{
			NEXT_ARG();
			if(get_unsigned(&other.batch,*argv,0) || other.batch >100 || other.batch < 10)
				invarg("Invalid \"batch\" value\n",*argv);
			if(mode == CLIENT )
				invarg("Only valid in server mode.\n",*argv);
			else
			{
				
			}
			
		}else if (matches(*argv,"idletimeout") == 0 || matches(*argv,"idle") == 0)
		{
			NEXT_ARG();
		}else if( matches(*argv,"mode") == 0)
		{
			NEXT_ARG();
			if(strcmp(*argv,"client") == 0 )
			{
				mode = NW_MODE_CLIENT;
			}else if (strcmp(*argv,"server") == 0)
			{
				mode = NW_MODE_SERVER;
			}
		}
		else{
			if(strcmp(*argv,"dev") == 0 )
				NEXT_ARG();
			else if (strcmp(*argv,"help") == 0)
				nw_usage();
			if(dev)
			 	duparg2("dev",*argv);
			if(check_ifname(*argv))
				invarg("\"dev\" not a valid ifname",*argv);
			strcpy(&dev , *argv);
		}
		argc--;
		argv++;
	}
	if(!dev)
	{
		fprintf(stderr,"Not enough information:\"dev\" argument is required.\n");
		exit(-1);
	}

    return 0;
}
int nw_dev_del(int argc, char **argv)
{
    return 0;
}

int nw_dev_close(int argc, char **argv)
{
    return 0;
}