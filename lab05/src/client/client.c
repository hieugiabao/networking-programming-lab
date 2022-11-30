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

  struct addrinfo hints, *servinfo;
  int rv, sock_fd;

  printf("Starting client connecting to host: %s on port: %s\n", argv[1], argv[2]);

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  if ((sock_fd = socket(servinfo->ai_family, servinfo->ai_socktype,
                        servinfo->ai_protocol)) == -1)
  {
    perror("client: socket");
    exit(EXIT_FAILURE);
  }
  if (connect(sock_fd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
  {
    close(sock_fd);
    perror("client: connect");
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(servinfo);

  printf("Connected to server\n");
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

    bzero(buffer, BUF_SIZE);
    sprintf(buffer, "%s%c%s%c", username, 0, password, 0);
    // sent to server
    bytes_sent = send(sock_fd, buffer, strlen(username) + strlen(password) + 2, 0);
    if (bytes_sent == -1)
    {
      perror("Error occured: Can't send data to server");
      exit(EXIT_FAILURE);
    }

    // receive from server
    bytes_recv = recv(sock_fd, buffer, BUF_SIZE, 0);
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
        bytes_sent = send(sock_fd, buffer, strlen(buffer), 0);
        if (bytes_sent == -1)
        {
          perror("Error occured1: Can't send data to server");
          exit(EXIT_FAILURE);
        }
        // print new address host and port

        char *sent = strdup(buffer);

        bytes_recv = recv(sock_fd, buffer, BUF_SIZE, 0);
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
    }
  }
  close(sock_fd);
}
