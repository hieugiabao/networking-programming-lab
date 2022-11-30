#include "resolver.h"

char *ipaddr_to_domain(char *ipaddr, char *domains)
{
  struct in_addr addr;
  inet_aton(ipaddr, &addr);
  struct hostent *host = gethostbyaddr(&addr, sizeof addr, AF_INET);
  if (host == NULL)
  {
    return NULL;
  }
  strcpy(domains, host->h_name);
  return domains;
}

char *domain_to_ipaddr(char *domain, char *ipaddrs)
{
  struct hostent *host = gethostbyname(domain);
  if (host == NULL)
  {
    return NULL;
  }
  strcpy(ipaddrs, inet_ntoa(*((struct in_addr *)host->h_addr)));
  return ipaddrs;
}

int is_valid_domain(const char *domain)
{
  regex_t regex;
  int reti = regcomp(&regex, "^([A-Za-z0-9-]{1,63}.)+[A-Za-z]{2,6}$", REG_EXTENDED);
  reti = regexec(&regex, domain, 0, NULL, 0);

  if (reti == REG_NOMATCH)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

int is_valid_ipaddr(const char *ipaddr)
{
  struct sockaddr_in sa;
  int result = inet_pton(AF_INET, ipaddr, &(sa.sin_addr));
  return result != 0;
}
