#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "status.h"
#include "utils.h"

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int sock_fd;
  struct sockaddr_in server_address, new_server_address;
  socklen_t server_len, new_server_len;

  bzero(&server_address, sizeof(server_address));

  if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    perror("Error occured: Can't create socket");
    exit(EXIT_FAILURE);
  }

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(atoi(argv[2]));
  server_address.sin_addr.s_addr = inet_addr(argv[1]);

  server_len = sizeof(server_address);
  new_server_len = sizeof(new_server_len);

#define BUF_SIZE 4096

  int bytes_sent, bytes_recv;
  char username[100];
  char password[100];
  char buffer[BUF_SIZE];

  while (1)
  {
    printf("Username: ");
    scanf("%s", username);
    while (getchar() != '\n')
      ;
    printf("Insert password: ");
    get_password(password, sizeof password);

    sprintf(buffer, "%s%c%s%c", username, 0, password, 0);
    // sent to server
    bytes_sent = sendto(sock_fd, buffer, strlen(username) + strlen(password) + 2, 0, (struct sockaddr *)&server_address, server_len);

    if (bytes_sent == -1)
    {
      perror("Error occured: Can't send data to server");
      exit(EXIT_FAILURE);
    }

    // receive from server
    bytes_recv = recvfrom(sock_fd, buffer, BUF_SIZE, 0, (struct sockaddr *)&new_server_address, &new_server_len);
    buffer[bytes_recv] = '\0';

    if (bytes_recv == -1)
    {
      perror("Error occured: Can't receive data from server");
      exit(EXIT_FAILURE);
    }

    int status = atoi(buffer);
    if (status == SUCCESS)
    {
      printf("OK\n");
      while (1)
      {
        printf("Enter your message: ");
        fgets(buffer, BUF_SIZE, stdin);
        // trim \n
        buffer[strlen(buffer) - 1] = '\0';
        bytes_sent = sendto(sock_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&new_server_address, new_server_len);
        if (bytes_sent == -1)
        {
          perror("Error occured1: Can't send data to server");
          exit(EXIT_FAILURE);
        }
        // print new address host and port

        char *sent = strdup(buffer);

        bytes_recv = recvfrom(sock_fd, buffer, BUF_SIZE, 0, (struct sockaddr *)NULL, NULL);
        buffer[bytes_recv] = '\0';

        if (bytes_recv == -1)
        {
          perror("Error occured: Can't receive data from server");
          exit(EXIT_FAILURE);
        }

        if (strcmp(sent, "bye") == 0)
        {
          printf("Goodbye %s\n", username);
          break;
        }

        char *strpt = buffer;
        int status = atoi(strpt);
        strpt += strlen(strpt) + 1;

        if (status == SUCCESS)
        {
          char *numeric = malloc(100);
          char *alphabet = malloc(100);

          parse_buffer(strpt, numeric, alphabet);
          if (strlen(numeric) > 0)
          {
            printf("%s\n", numeric);
          }
          if (strlen(alphabet) > 0)
          {
            printf("%s\n", alphabet);
          }
        }
        else
        {
          printf("Error\n");
        }
      }
    }
    else
    {
      error(status);
      continue;
    }
  }
}
