#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#else
#include <arpa/inet.h>
#include <netdb.h>
#include <regex.h>
#define REGEX "prefix:\\w+,\\w+,\\s*-?[0-9]{1,4}\\s*,\\s*-?[0-9]{1,4}\\s*,\\s*-?[0-9]{1,4}\\s*,\\w*" // regex for the prefix

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{

  if (argc < 3 || (strcmp(argv[1], "1") != 0 && strcmp(argv[1], "2") != 0))
  {
    fprintf(stderr, "usage: resolver 1 ipaddress\n");
    fprintf(stderr, "usage: resolver 2 hostname\n");
    return 1;
  }

#if defined(_WIN32)
  WSADATA d;
  if (WSAStartup(MAKEWORD(2, 2), &d))
  {
    fprintf(stderr, "Failed to initialize.\n");
    return 1;
  }
#endif

  switch (atoi(argv[1]))
  {
  case 1:
  {
    // check if it a ip address with regex
    regex_t regex;
    int reti;
    reti = regcomp(&regex, "^([0-9]{1,3}.)+[0-9]{1,3}$", REG_EXTENDED);
    reti = regexec(&regex, argv[2], 0, NULL, 0);
    if (reti == REG_NOMATCH)
    {
      fprintf(stderr, "Wrong parameter\n");
      return 1;
    }
    struct in_addr addr;
    inet_aton(argv[2], &addr);
    struct hostent *he = gethostbyaddr(&addr, sizeof(addr), AF_INET);
    if (he == NULL)
    {
      fprintf(stderr, "Not found information\n");
      return 1;
    }

    printf("Official name is: %s\n", he->h_name);
    // get alias name:
    char **alias = he->h_aliases;
    while (*alias != NULL)
    {
      printf("Alias name is: %s\n", *alias);
      alias++;
    }
  }
  break;
  case 2:
  {
    // check if it a domain name with regex
    regex_t regex;
    int reti;
    reti = regcomp(&regex, "^([A-Za-z0-9-]{1,63}.)+[A-Za-z]{2,6}$", REG_EXTENDED);
    reti = regexec(&regex, argv[2], 0, NULL, 0);
    if (reti == REG_NOMATCH)
    {
      fprintf(stderr, "Wrong parameter\n");
      return 1;
    }
    struct hostent *he = gethostbyname(argv[2]);
    if (he == NULL)
    {
      fprintf(stderr, "Not found information\n");
      return 1;
    }
    printf("Offical IP: %s\n", inet_ntoa(*(struct in_addr *)he->h_addr));
    struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;
    if (addr_list[1] != NULL)
    {
      printf("Aliases address:\n");
      for (int i = 1; addr_list[i] != NULL; i++)
      {
        printf("%s\n", inet_ntoa(*addr_list[i]));
      }
    }
  }
  break;
  default:
    break;
  }

#if defined(_WIN32)
  WSACleanup();
#endif
  return 0;
}
