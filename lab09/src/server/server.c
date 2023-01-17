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
#include <fcntl.h>
#include <sys/poll.h>
#include <signal.h>

#include "file.h"
#include "list.h"
#include "status.h"
#include "utils.h"

#define FILE_PATH "account.txt"

struct client_info
{
  socklen_t address_length;
  struct sockaddr_in address;
  int socket;
  char is_login;
  char username[MAX_USERNAME_LEN + 1];
  int wrong_password_times;
  struct client_info *next;
};

static struct client_info *clients = 0;

int init_server(const char *port);
void run_server(int sock_fd);
void split_password(const char *password, char *numeric, char *alphabet);
struct client_info *get_client(int s);
void drop_client(struct client_info *client);

void sigintHandler()
{
  printf("closing connection\n");
  close_input_stream();
  exit(0);
}

int main(int argc, char const *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  signal(SIGINT, sigintHandler);

  if (open_input_stream(FILE_PATH) == IO_ERROR)
  {
    fprintf(stderr, "Error occured: Can't open file '%s' to load user infomation", FILE_PATH);
  }
  get_list_users_from_file();

  int sock_fd = init_server(argv[1]);
  run_server(sock_fd);

  close_input_stream();
  close(sock_fd);
  free_data();
  return 0;
}

int init_server(const char *port)
{
  int sock_fd;
  struct sockaddr_in server_address;
  bzero(&server_address, sizeof(server_address));

  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Error occured: Can't create socket");
    exit(EXIT_FAILURE);
  }

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(atoi(port));
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sock_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
  {
    perror("Error occured: Can't bind socket");
    exit(EXIT_FAILURE);
  }

  if (listen(sock_fd, 5) == -1)
  {
    perror("Error occured: Can't listen");
    exit(EXIT_FAILURE);
  }

  printf("Server is listening on port %s\n", port);

  return sock_fd;
}

void run_server(int sock_fd)
{
#define MAX_PASSWORD_WRONG 3
#define BUF_SIZE 4096

  int bytes_recv;
  char buffer[BUF_SIZE];

  struct pollfd fds[100];
  int fds_size = 0;
  fds[fds_size++].fd = sock_fd;
  fds[fds_size - 1].events = POLLIN;

  printf("Waiting for connections...\n");

  while (1)
  {

    if (poll(fds, fds_size, 0) < 0)
    {
      perror("Error occured: Can't poll");
      return;
    }

    int i;
    for (i = 0; i < fds_size; i++)
    {
      if (fds[i].revents && POLLIN)
      {
        if (fds[i].fd == sock_fd)
        {
          struct client_info *client = get_client(-1);
          client->socket = accept(sock_fd, (struct sockaddr *)&(client->address), &(client->address_length));
          if (client->socket < 0)
          {
            perror("Error occured: Can't accept");
            return;
          }

          fds[fds_size++].fd = client->socket;
          fds[fds_size - 1].events = POLLIN;

          printf("New connection from %s:%d\n", inet_ntoa(client->address.sin_addr), ntohs(client->address.sin_port));
        }
        else
        {
          struct client_info *client = get_client(fds[i].fd);
          char read[1024];
          int bytes_received = recv(client->socket, read, 1024, 0);
          read[bytes_received] = '\0';

          if (bytes_received < 1)
          {
            printf("Connection from %s:%d closed\n", inet_ntoa(client->address.sin_addr), ntohs(client->address.sin_port));
            drop_client(client);

            for (int j = i; j < fds_size - 1; j++)
            {
              fds[j] = fds[j + 1];
            }
            fds_size--;

            continue;
          }

          if (client->is_login == 0)
          {
            char username[MAX_HOMEPAGE_LEN + 1];
            char password[MAX_PASSWORD_LEN + 1];
            parse_buffer(read, username, password);

            int status = login(username, password);
            if (status != SUCCESS && status == ERR_PASSWORD_INCORRECT)
            {
              if (strcmp(username, client->username) != 0)
              {
                client->wrong_password_times = 1;
                strcpy(client->username, username);
              }
              else
              {
                client->wrong_password_times++;
                if (client->wrong_password_times >= MAX_PASSWORD_WRONG)
                {
                  change_status(username, BLOCKED);
                  status = ERR_PASSWORD_INCORRECT_OVER;
                }
              }
            }

            if (status == SUCCESS)
            {
              client->is_login = 1;
              strcpy(client->username, username);
              client->wrong_password_times = 0;
            }

            char *message = (char *)malloc(1024);
            sprintf(message, "%d", status);
            send(client->socket, message, strlen(message), 0);
          }
          else
          {
            if (strcmp(read, "bye") == 0)
            {
              int response = logout(client->username);
              sprintf(read, "%d", response);
              send(client->socket, read, strlen(read), 0);
              client->is_login = 0;
              continue;
            }
            else
            {
              if (is_valid_password(read))
              {
                change_password(client->username, read);
                char *numeric = (char *)malloc(MAX_PASSWORD_LEN + 1);
                char *alpha = (char *)malloc(MAX_PASSWORD_LEN + 1);

                split_password(read, numeric, alpha);
                sprintf(read, "%d%c%s%c%s%c", SUCCESS, 0, numeric, 0, alpha, 0);
                send(client->socket, read, strlen(alpha) + strlen(numeric) + 6, 0);
              }
              else
              {
                sprintf(read, "%d", ERR_PASSWORD_FORMAT_INVALID);
                send(client->socket, read, strlen(read), 0);
              }
            }
          }
        }
      }
    }
  }
}

void split_password(const char *password, char *numeric, char *alphabet)
{
  int i, j, k;
  for (i = 0, j = 0, k = 0; i < strlen(password); i++)
  {
    if (isdigit(password[i]))
    {
      numeric[j++] = password[i];
    }
    else if (isalpha(password[i]))
    {
      alphabet[k++] = password[i];
    }
  }
  numeric[j] = '\0';
  alphabet[k] = '\0';
}

struct client_info *get_client(int s)
{
  struct client_info *ci = clients;

  while (ci)
  {
    if (ci->socket == s)
      break;
    ci = ci->next;
  }

  if (ci)
    return ci;
  struct client_info *n =
      (struct client_info *)calloc(1, sizeof(struct client_info));

  if (!n)
  {
    fprintf(stderr, "Out of memory.\n");
    exit(1);
  }

  n->address_length = sizeof(n->address);
  n->wrong_password_times = 0;
  n->is_login = 0;
  n->next = clients;
  clients = n;
  return n;
}

void drop_client(struct client_info *client)
{
  close(client->socket);

  struct client_info **p = &clients;

  while (*p)
  {
    if (*p == client)
    {
      *p = client->next;
      free(client);
      return;
    }
    p = &(*p)->next;
  }

  fprintf(stderr, "drop_client not found.\n");
  exit(1);
}
