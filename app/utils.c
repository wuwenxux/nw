#include "utils.h"
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include "nw_err.h"
#include <err.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <arpa/inet.h>	
int get_ip_mask(const char *dev,char *ip_str,char *mask)
{
	struct ifaddrs *ifaddr, *ifa;
   // int s;

	assert(dev !=NULL);
    if (getifaddrs(&ifaddr) == -1) 
    {
        perror("getifaddrs");
       	return -1;
    }
    
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if((ifa->ifa_addr != NULL) && 
			(ifa->ifa_netmask != NULL) &&
            (strcmp(ifa->ifa_name, dev) == 0) &&  
            (ifa->ifa_addr->sa_family == AF_INET)) 
		{
			strcpy(ip_str, inet_ntoa(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr));
			strcpy(mask,inet_ntoa(((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr));
			break;
        }
		strcpy(ip_str,"0.0.0.0");
		strcpy(mask,"0.0.0.0");
	
    }
    freeifaddrs(ifaddr);
	return 0;

}
int check_nw_if(const char *str)
{
	FILE	*fp;
	char	buf[256];
	char	*cmdline = "ip link";
	char	*i, *n;


	if (strlen(str) >= IFNAMSIZ) {
		return INVALID_ARG;
	}

	if ((fp=popen(cmdline,"r")) == NULL) {
		err(EXIT_FAILURE, "%s", cmdline);
		return CMDERR;
	}

	memset(buf, 0, sizeof(buf));

	/* index name search */
	while(fgets(buf, 256, fp) != NULL) {
		if (*buf != ' ') {
			i = strtok(buf, ":");
			if (i == NULL) {
				pclose(fp);
				return -1;
			}
			n = strtok(NULL, ":");
			if (n == NULL) {
				pclose(fp);
				return -1;
			}
			if (strcmp(&n[1], str) == 0) {
				pclose(fp);
				return 0;
			}
		}
	}
	pclose(fp);
	/* not found */
	return DEV_NOT_FOUND;
}
int nw_mptcp(const char *str)
{
	FILE *fp;
	char buf[256];
	char cmd[100];
	
	if (strlen(str) >= IFNAMSIZ || check_ifname(str)) {
		return INVALID_ARG;
	}
	sprintf(cmd,"ip link show dev %s |grep -o NOMULTIPATH",str);
	if ((fp = popen(cmd,"r")) == NULL) {
		err(EXIT_FAILURE, "%s", cmd);
		return CMDERR;
	}
	while(fgets(buf, 256, fp) != NULL)
	{
		if(*buf != ' ')
		{
			if(strncmp(buf,"NOMULTIPATH",11) == 0)
			{
				pclose(fp);
				return 0;
			}
		}
	}
	pclose(fp);
	return -1;
}

static int __check_ifname(const char *name)
{
	if (*name == '\0')
		return -1;
	while (*name) {
		if (*name == '/' || isspace(*name))
			return -1;
		++name;
	}
	return 0;
}
void invarg(const char *msg, const char *arg)
{
	fprintf(stderr, "Error: argument \"%s\" is wrong,%s\n", arg, msg);
	return;
}

int check_ifname(const char *name)
{
	/* These checks mimic kernel checks in dev_valid_name */
	if (strlen(name) >= IFNAMSIZ)
		return -1;
	return __check_ifname(name);
}
bool check_maxbufflen(uint32_t maxbuf)
{
	if( maxbuf < 64 || maxbuf > 102400)
		return false; 
	return true;
}
void duparg(const char *key, const char *arg)
{
	fprintf(stderr,
		"Error: duplicate \"%s\": \"%s\" is the second value.\n",
		key, arg);
	return;
}
void duparg2(const char *key, const char *arg)
{
	fprintf(stderr,
		"Error:  \"%s\" is a not a parameter of %s.\n",
		arg,key);
	return;
}
int get_unsigned32(unsigned int *val, const char *arg, int base)
{
	unsigned long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;

	res = strtoul(arg, &ptr, base);

	/* empty string or trailing non-digits */
	if (!ptr || ptr == arg || *ptr)
		return -1;

	/* overflow */
	if (res == UINT_MAX && errno == ERANGE)
		return -1;

	/* out side range of unsigned short*/
	if (res > UINT_MAX)
		return -1;

	*val = res;
	return 0;
}
int check_filepath(char *path)
{
	FILE *fp;
	assert(path != NULL);
	if ((fp = fopen(path, "r")) == NULL) {
		return FILE_NOT_FOUND;
	}
	return 0;
}
int check_ipv4(const char *str)
{
	int digit, i, pos;
	char addr_tmp[IPV4NAMESIZE];
	char *ip_addr_p, *save_p, *p;

	memset(addr_tmp, 0, sizeof(addr_tmp));

	strcpy(addr_tmp, str);

	if(strlen(str) >= IPV4NAMESIZE)
		return IPV4ERR;
	for (digit = i = pos = 0; addr_tmp[i]; i++) {
		if (isdigit(addr_tmp[i]) != 0) {
			digit++;
		} else {
			if ((addr_tmp[i] == '.') && (digit > 0) && (digit < 4) && (pos < 4)) {
				digit = 0;
				pos++;
			} else {
				return IPV4ERR;
			}
		}
	}

	if (pos != 3) {
		return IPV4ERR;
	}


	for (i = 0, p = &addr_tmp[0]; i < 4; i++, p = NULL) {
		if ((ip_addr_p = (char *)strtok_r(p, ".", &save_p)) == NULL) {
			return IPV4ERR;
		}

		if ((0 > atoi(ip_addr_p)) || (atoi(ip_addr_p) > 255)) {
			return IPV4ERR;
		}
	}

	return 0;
}
int check_netmask(const char *mask)
{
	unsigned int n_mask;
	unsigned int h_mask;
	inet_pton(AF_INET,mask,&n_mask);
	h_mask = ntohl(n_mask);
	if(h_mask == UINT32_MAX || h_mask == 0)
		return 0;
	else if(h_mask & (~h_mask >> 1))
		return -1;
	else 
		return 0;
}
int get_unsigned16(unsigned short *val, const char *arg, int base)
{
	unsigned long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;

	res = strtoul(arg, &ptr, base);

	/* empty string or trailing non-digits */
	if (!ptr || ptr == arg || *ptr)
		return -1;

	/* overflow */
	if (res == USHRT_MAX && errno == ERANGE)
		return -1;

	/* out side range of unsigned short*/
	if (res > USHRT_MAX)
		return -1;

	*val = res;
	return 0;
}
/*convert from char * to int value */
int get_integer(int *val, const char *arg, int base)
{
	long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;

	res = strtol(arg, &ptr, base);

	/* If there were no digits at all, strtol()  stores
	 * the original value of nptr in *endptr (and returns 0).
	 * In particular, if *nptr is not '\0' but **endptr is '\0' on return,
	 * the entire string is valid.
	 */
	if (!ptr || ptr == arg || *ptr)
		return -1;

	/* If an underflow occurs, strtol() returns LONG_MIN.
	 * If an overflow occurs,  strtol() returns LONG_MAX.
	 * In both cases, errno is set to ERANGE.
	 */
	if ((res == LONG_MAX || res == LONG_MIN) && errno == ERANGE)
		return -1;

	/* Outside range of int */
	if (res < INT_MIN || res > INT_MAX)
		return -1;

	*val = res;
	return 0;
}
/* Returns false if 'prefix' is a not empty prefix of 'string'.*/
bool matches(const char *prefix, const char *string)
{
	if (!*prefix)
		return true;
	while (*string && *prefix == *string) {
		prefix++;
		string++;
	}

	return !!*prefix;
}

void blank_del(char *buf)
{
	int i, j, len, b_flag;
	char t_buf[BUFLEN];

	memset(t_buf, 0, sizeof(t_buf));

	for (b_flag = i = j = 0, len = strlen(buf); i < len; i++) {
		/**
		 * isspace handles below:
		 * form-feed ('\f'),
		 * newline ('\n'), 
		 * carriage return ('\r'),
		 * horizontal tab  ('\t'), 
		 * and  vertical  tab
         * ('\v').
		 */
		if (isspace(buf[i])) {
			if (b_flag == 0) {
				continue;
			} else {
				b_flag = 1;
				if (buf[i+1] == '\0') {
					break;
				}
				if (isspace(buf[i+1])) {
					continue;
				} else {
					t_buf[j] = buf[i];
					j++;
				}
			}
		} else {
			b_flag = 1;
			t_buf[j] = buf[i];
			j++;
		}
	}
	memcpy(buf, t_buf, sizeof(t_buf));
}
void trim(char *source_string, const char *trim_string)
{	
	int input_len = strlen(source_string);
	int trim_string_len = strlen(trim_string);
	char *input_value_ptr = source_string;
	const char *trim_string_ptr = trim_string; 
 
	bool escape_flag = false;
	int i = 0, j = 0;
 
	assert(source_string != NULL);
	assert(trim_string != NULL);
 
	/* delete the front character */
	for (i = 0; i < input_len; i++)
	{
			/* cycle the trim string, if the character isn't one of trim string, 
			 * escape the cycle, and move the string */
			for (j = 0; j < trim_string_len; j++)
			{
				if (input_value_ptr[i] == trim_string_ptr[j])
				{
					break;
				}
				if(j == (trim_string_len - 1))
				{
					escape_flag = true;
				}
			}	
			if(escape_flag)
			{
				break;
			}
	}	
	memmove(input_value_ptr, input_value_ptr+i,(input_len - i + 1)); 
	input_len = strlen(input_value_ptr);
	escape_flag = false;
	/* delete the end character */
	for (i = input_len - 1; i >= 0; i--)
	{
		/* cycle the trim string, if the character isn't one of trim string, 
		 * escape the cycle, and move the string */
		for (j = 0; j < trim_string_len; j++)
		{
			if (input_value_ptr[i] == trim_string_ptr[j])
			{
				break;
			}
			if(j == (trim_string_len - 1))
			{
				escape_flag = true;
			}
		}	
		if(escape_flag)
		{
			break;
		}
	}	
	input_value_ptr[i+1] = '\0';
 	return;
}
int nw_dev_on(const char *dev)
{
	assert(dev != NULL);
	assert(check_ifname(dev) == 0);
	int ret = -1;
	char set_on_cmd[50];
	sprintf(set_on_cmd,"ip link set %s up",dev);
	ret = system(set_on_cmd);
	return ret == 0 ? 0 : -1;
}