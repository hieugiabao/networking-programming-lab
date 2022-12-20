#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>

#include "file.h"
#include "list.h"
#include "status.h"
#include "utils.h"

#define FILE_PATH "account.txt"

int init_server(const char *port);
void run_server(int sock_fd);
void handle_client(int sock_client, fd_set master, int *);
void split_password(const char *password, char *numeric, char *alphabet);

void sigintHandler()
{
  printf("closing connection\n");
  close_input_stream();
  exit(0);
}

void sigchld_handler(int s)
{
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
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
  struct sigaction sa;
  int yes = 1;
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

  sa.sa_handler = sigchld_handler; // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1)
  {
    perror("sigaction");
    exit(1);
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

  printf("Waiting for connections...\n");

  while (1)
  {
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    int sock_client = accept(sock_fd, (struct sockaddr *)&client_address, &client_len);
    if (sock_client < 0)
    {
      perror("Error occured: Can't accept");
      exit(EXIT_FAILURE);
    }
    printf("New connection from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

    if (!fork())
    {
      close(sock_fd);
      fd_set master;
      int worng_password_times = 0;
      FD_ZERO(&master);
      FD_SET(sock_client, &master);
      while (FD_ISSET(sock_client, &master))
      {
        handle_client(sock_client, master, &worng_password_times);
      }
      close(sock_client);
      exit(EXIT_SUCCESS);
    }
  }
}

void handle_client(int sock_client, fd_set master, int *wrong_password_times)
{
  int is_login = 0;
  int bytes_recv, bytes_sent;

  char mess[BUF_SIZE];
  char *username = (char *)malloc(MAX_USERNAME_LEN + 1),
       *password = (char *)malloc(MAX_PASSWORD_LEN + 1);

  fd_set read;
  read = master;
  if (select(sock_client + 1, &read, NULL, NULL, NULL) == -1)
  {
    perror("Error occured: Can't select");
    exit(EXIT_FAILURE);
  }

  if (FD_ISSET(sock_client, &read))
  {
    bytes_recv = recv(sock_client, mess, BUF_SIZE, 0);
    if (bytes_recv < 1)
    {
      FD_CLR(sock_client, &master);
      printf("Connection closed by client\n");
      close(sock_client);
      exit(EXIT_SUCCESS);
    }
    // parse buffer
    parse_buffer(mess, username, password);
    int response = login(username, password);

    if (response != SUCCESS && response == ERR_PASSWORD_INCORRECT)
    {
      if (*wrong_password_times >= MAX_PASSWORD_WRONG)
      {
        change_status(username, BLOCKED);
        *wrong_password_times = 0;
        response = ERR_PASSWORD_INCORRECT_OVER;
      }
    }

    sprintf(mess, "%d", response);
    send(sock_client, mess, strlen(mess), 0);

    if (response == SUCCESS)
    {
      is_login = 1;
      *wrong_password_times = 0;
    }
    else
    {
      *wrong_password_times += 1;
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
