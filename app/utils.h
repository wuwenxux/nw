#ifndef _UTIL_H_
#define _UTIL_H_
/* util */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#define IPV4NAMESIZE 20
#define BUFLEN 512
#define NEXT_ARG() \
	do { \
		argv++; \
		if (--argc < 0) { \
			fprintf(stderr, "missing parameter for %s\n", *argv); \
			exit(EXIT_FAILURE);\
		}\
	} while(0)

#define NEXT_ARG_OK() (argc - 1 > 0)
#define NEXT_ARG_FWD() do { argv++; argc--; } while(0)
#define PREV_ARG() do { argv--; argc++; } while(0)

int yes_no(const char *msg, const char *realval);
/*util*/
/**
 * Convert  from a u32 value to a client/server str.
 * Return: client/server  if not exist NULL.
 * @mode:1 client 2 server 
 **/
int cli_ser(const char *msg, const char *realval);
int on_off(const char *,const char *);
int check_self(const char *);
/**
 * Note:not a valid argu message
 * @msg:error promopt
 * @arg:*argv value
 */
void invarg(const char *msg, const char *arg);
/**
 * Like strcmp ,but some corner cases have been considered
 * Return :true if match ,else false
 * @prefix:the substr of str
 * @str: input str 
 **/
bool matches(const char *prefix,const char *str);
/**
 * Get a u16 value from char * str.
 * Return : 0 success ,-1 failure(out of range ,empty str,overflow);
 * @val:to store the str to a unsigned short val
 * @arg:input str
 * @base: default decimal ,10
 **/
int get_unsigned16(unsigned short *val, const char *arg, int base);
/**
 * Get a u32 value from char * str.
 * Return : 0 success ,-1 failure,;
 * @val:to store the str to a unsigned int val
 * @arg:input str
 * @base: default decimal,10 
 **/
int get_unsigned32(unsigned int *val, const char *arg, int base);
/**
 * A param is duplicate
 * Note:key is legal ,and arg is duplicate.
 * @key:a existed key
 * @arg:second value
 **/
void duparg(const char *key, const char *arg);
/**
 * Ether key or arg is duplicate.
 * Note:we can not determine which one is valid ,either key is useless or arg
 * @key:first value
 * @arg:second value
 **/
void duparg2(const char *key, const char *arg);
/**
 * Whether name is a valid ifname 
 *Return:0 SUCCESS ,-1 FAIL
 *@name: name of the dev 
 **/
int check_ifname(const char *name);
/**
 *Check a str is a valid ipv4 address
 *Return: 0 SUCCESS , -1 Failed
 *@ipv4_str: a str of ipv4
 */
int check_ipv4(const char *ipv4_str);
/**
 * Check a mask is valid 
 * Return: 0 Success ,-1 Failed.
 * @ipmask_str:a str like 255.255.168.0
 **/
int check_netmask(const char *ipmask_str);
int get_ip_mask(const char *dev,char *ip_str,char *mask );
/**
 * Whether a dev is a valid nw interface.
 * Note:Dev is not a ngmwan dev 
 * Return:if exist return 0,else return -1 
 * @dev:a valid dev name
 * Note: name of dev must be valid.
 **/
int check_nw_if(const char *dev);
/**
 * To confirm whether multipath is supported by dev
 * Note: dev should be a effective name ,otherwise  DEVERR , CMDERR,-1 returned.
 * Return: 0 SUCCESS else DEVERR, CMDERR, -1 FAILED
 * @dev:name of the device ,which can be found by ip link.
 **/
int nw_mptcp_read(const char *dev);
/**
 * Make a dev on (show in ifconfig)
 * Note: dev should be a effective name ,otherwise  DEVERR , CMDERR,-1 returned.
 * Return: 0 SUCCESS -1 FAILED
 * @dev:name of the device ,which can be found by ip link.
 **/
int nw_dev_on(const char*dev);
/**
 *Trim a string with trim characters like "\'\" "
 *Note: option "hello" --> option hello
 *@line: a line of str(512 chars)
 *@trim_sep:"\"\' " ,default ,refer to trim_str
 **/
void trim(char *line, const char *trim_sep);
/**
 *Make a line more origanized
 *Note:line of chars '\t','\r','\v','\n' '  ' --> ' '
 *@line: a line of chars
 **/
void blank_del(char *line);
/**
 *Whether a file path is valid.
 *Return :0 SUCCESS ,FILE_NOT_FOUND FAILED
 *@path:whole path to a file.
 **/ 
int check_filepath(char *path);
#endif 