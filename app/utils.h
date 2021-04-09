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
 * Return : 0 success ,-1 failure;
 * @val:to store the str to a unsigned short val
 * @arg:input str
 * @base: default decimal 
 **/
int get_unsigned16(unsigned short *val, const char *arg, int base);
int get_unsigned32(unsigned int *val, const char *arg, int base);
void duparg(const char *key, const char *arg);
void duparg2(const char *key, const char *arg);
int check_ifname(const char *name);
int check_ipv4(const char *);
int check_nw_if(char *);
int nw_mptcp(char *);
void trim(char *, const char *);
void blank_del(char *);
int check_filepath(char *);
#endif 