#ifndef __RESOLVER_H__
#define __RESOLVER_H__

#include <arpa/inet.h>
#include <netdb.h>
#include <regex.h>
#define REGEX "prefix:\\w+,\\w+,\\s*-?[0-9]{1,4}\\s*,\\s*-?[0-9]{1,4}\\s*,\\s*-?[0-9]{1,4}\\s*,\\w*" // regex for the prefix

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *ipaddr_to_domain(char *ipaddr, char *domains);
char *domain_to_ipaddr(char *domain, char *ipaddrs);
int is_valid_domain(const char *domain);
int is_valid_ipaddr(const char *ipaddr);

#endif
