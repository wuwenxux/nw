#include "nw_cli.h"
#include "utils.h"
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

int nw_dev_show(int argc, char **argv)
{
    int ret;
    
    return 0;
}
int nw_dev_bindport(int argc, char **argv)
{
    return 0;
}
int nw_dev_set(int argc, char **argv)
{
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