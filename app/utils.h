#ifndef _UTIL_H_
#define _UTIL_H_
/* util */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#define NEXT_ARG() do { argv++; if (--argc <= 0) __attribute__((noreturn)); } while(0)
#define NEXT_ARG_OK() (argc - 1 > 0)
#define NEXT_ARG_FWD() do { argv++; argc--; } while(0)
#define PREV_ARG() do { argv--; argc++; } while(0)
void invarg(const char *msg, const char *arg);
bool matches(const char *prefix,const char *str);
int get_unsigned(unsigned int *val, const char *arg, int base);
void duparg(const char *key, const char *arg);
void duparg2(const char *key, const char *arg);
int check_ifname(const char *name);
#endif 