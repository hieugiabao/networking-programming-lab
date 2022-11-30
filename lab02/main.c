#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int isIPAdr(char *ip)
{
  char *token = strtok(ip, ".");
  int i, length;
  length = strlen(token);
  for (i = 0; i < length; i++)
  {
    if (!isdigit(token[i]))
    {
      return 0;
    }
  }
  return 1;
}

int main(int argc, char *argv[])
{

  if (argc < 3 || (strcmp(argv[1], "1") != 0 && strcmp(argv[1], "2") != 0))
  {
    fprintf(stderr, "usage: resolver 1 ipaddress\n");
    fprintf(stderr, "usage: resolver 2 hostname\n");
    return 1;
  }

  char homepage[30];
  strcpy(homepage, argv[2]);

  switch (atoi(argv[1]))
  {
  case 1:
  {
    if (isIPAdr(homepage) == 0)
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
    char **alias = he->h_addr_list;
    while (*alias != NULL)
    {
      printf("Alias name is: %s\n", *alias);
      alias++;
    }
  }
  break;
  case 2:
  {
    if (isIPAdr(homepage) == 1)
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
  return 0;
}
