#include "utils.h"
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <net/if.h>

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

int check_ifname(const char *name)
{
	/* These checks mimic kernel checks in dev_valid_name */
	if (strlen(name) >= IFNAMSIZ)
		return -1;
	return __check_ifname(name);
}

void duparg(const char *key, const char *arg)
{
	fprintf(stderr,
		"Error: duplicate \"%s\": \"%s\" is the second value.\n",
		key, arg);
	exit(-1);
}

void duparg2(const char *key, const char *arg)
{
	fprintf(stderr,
		"Error: either \"%s\" is duplicate, or \"%s\" is a garbage.\n",
		key, arg);
	exit(-1);
}


int get_unsigned(unsigned int *val, const char *arg, int base)
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
	if (res == ULONG_MAX && errno == ERANGE)
		return -1;

	/* out side range of unsigned */
	if (res > UINT_MAX)
		return -1;

	*val = res;
	return 0;
}
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
