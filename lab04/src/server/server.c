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
#include <signal.h>

#include "file.h"
#include "list.h"
#include "status.h"
#include "utils.h"

#define FILE_PATH "account.txt"

int init_server(const char *port);
void run_server(int sock_fd);
void handle_client(const char *buffer, struct sockaddr_in client_address, socklen_t client_len);
void split_password(const char *password, char *numeric, char *alphabet);

void sigintHandler()
{
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
  free_data();
  return 0;
}

int init_server(const char *port)
{
  int sock_fd;
  struct sockaddr_in server_address;
  bzero(&server_address, sizeof(server_address));

  if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
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

  return sock_fd;
}

void run_server(int sock_fd)
{
#define MAX_PASSWORD_WRONG 3
#define BUF_SIZE 4096

  struct sockaddr_in client_address;
  socklen_t client_len = sizeof(client_address);
  int bytes_recv;
  char buffer[BUF_SIZE];

  while (1)
  {
    bytes_recv = recvfrom(sock_fd, buffer, BUF_SIZE, 0, (struct sockaddr *)&client_address, &client_len);
    if (bytes_recv == -1)
    {
      perror("Error occured: Can't receive data");
      exit(EXIT_FAILURE);
    }

    if (!fork())
    {
      close(sock_fd);
      // create new socket and bind
      handle_client(buffer, client_address, client_len);
    }
  }
}

void handle_client(const char *buffer, struct sockaddr_in client_address, socklen_t client_len)
{
  int new_sock_fd;
  int bytes_recv, bytes_sent;

  char mess[BUF_SIZE];
  char *username = (char *)malloc(MAX_USERNAME_LEN + 1),
       *password = (char *)malloc(MAX_PASSWORD_LEN + 1);

  if ((new_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    perror("Error occured: Can't create socket");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in new_ser_addr;
  bzero(&new_ser_addr, sizeof(new_ser_addr));

  new_ser_addr.sin_family = AF_INET;
  new_ser_addr.sin_port = htons(0);
  new_ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(new_sock_fd, (struct sockaddr *)&new_ser_addr, sizeof(new_ser_addr)) == -1)
  {
    perror("Error occured: Can't bind socket");
    exit(EXIT_FAILURE);
  }

  // parse buffer
  parse_buffer(buffer, username, password);
  int response = login(username, password);

  if (response != SUCCESS && response == ERR_PASSWORD_INCORRECT)
  {
    User *user = search_by_username(username);
    if (user->wrong_password_times >= MAX_PASSWORD_WRONG)
    {
      change_status(username, BLOCKED);
      user->wrong_password_times = 0;
      response = ERR_PASSWORD_INCORRECT_OVER;
    }
  }

  sprintf(mess, "%d", response);
  bytes_sent = sendto(new_sock_fd, mess, strlen(mess), 0, (struct sockaddr *)&client_address, client_len);
  if (bytes_sent == -1)
  {
    perror("Error occured: Can't send data");
    exit(EXIT_FAILURE);
  }

  if (response == SUCCESS)
  {
    // require change password
    while (1)
    {
      bytes_recv = recvfrom(new_sock_fd, mess, BUF_SIZE, 0, (struct sockaddr *)&client_address, &client_len);
      if (bytes_recv == -1)
      {
        perror("Error occured: Can't receive data");
        exit(EXIT_FAILURE);
      }

      mess[bytes_recv] = '\0';
      if (strcmp(mess, "bye") == 0)
      {
        int response = logout(username);
        sprintf(mess, "%d", response);
        bytes_sent = sendto(new_sock_fd, mess, strlen(mess), 0, (struct sockaddr *)&client_address, client_len);
        break;
      }
      else
      {
        if (is_valid_password(mess))
        {
          change_password(username, password, mess);
          char *numeric = (char *)malloc(MAX_PASSWORD_LEN + 1);
          char *alpha = (char *)malloc(MAX_PASSWORD_LEN + 1);

          split_password(mess, numeric, alpha);
          sprintf(mess, "%d%c%s%c%s%c", SUCCESS, 0, numeric, 0, alpha, 0);
          sendto(new_sock_fd, mess, strlen(alpha) + strlen(numeric) + 6, 0, (struct sockaddr *)&client_address, client_len);
        }
        else
        {
          sprintf(mess, "%d", ERR_PASSWORD_FORMAT_INVALID);
          sendto(new_sock_fd, mess, strlen(mess), 0, (struct sockaddr *)&client_address, client_len);
        }
      }
    }
  }

  close(new_sock_fd);
  exit(EXIT_SUCCESS);
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
