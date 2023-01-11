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
#include <pthread.h>

#include "file.h"
#include "list.h"
#include "status.h"
#include "utils.h"

#define FILE_PATH "account.txt"

struct entry
{
  char *username;
  int counts;
  struct entry *next;
};

void add_entry(struct entry **head, char *username)
{
  struct entry *new_entry = (struct entry *)malloc(sizeof(struct entry));
  new_entry->username = malloc(strlen(username) + 1);
  strcpy(new_entry->username, username);
  new_entry->counts = 1;
  new_entry->next = *head;
  *head = new_entry;
}

void plus_count(struct entry *head, char *username)
{
  struct entry *current = head;
  while (current != NULL)
  {
    if (strcmp(current->username, username) == 0)
    {
      current->counts++;
      return;
    }
    current = current->next;
  }
}

void reset_count(struct entry *head, char *username)
{
  struct entry *current = head;
  while (current != NULL)
  {
    if (strcmp(current->username, username) == 0)
    {
      current->counts = 0;
      return;
    }
    current = current->next;
  }
}

void free_entry(struct entry *head)
{
  struct entry *current = head;
  while (current != NULL)
  {
    struct entry *next = current->next;
    free(current->username);
    free(current);
    current = next;
  }
}

int times(struct entry *head, char *username)
{
  struct entry *current = head;
  while (current != NULL)
  {
    if (strcmp(current->username, username) == 0)
    {
      return current->counts;
    }
    current = current->next;
  }
  return -1;
}

int init_server(const char *port);
void run_server(int sock_fd);
void *handle_client(void *arg);
void split_password(const char *password, char *numeric, char *alphabet);

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

    int worng_password_times = 0;

    pthread_t tid;
    pthread_create(&tid, NULL, handle_client, &sock_client);
  }
}

void *handle_client(void *arg)
{
  int sock_client = *(int *)arg;
  int is_login = 0;
  int bytes_recv, bytes_sent;

  char mess[BUF_SIZE];
  char *username = (char *)malloc(MAX_USERNAME_LEN + 1),
       *password = (char *)malloc(MAX_PASSWORD_LEN + 1);
  struct entry *head = NULL;

  while (1)
  {
    bytes_recv = recv(sock_client, mess, BUF_SIZE, 0);
    if (bytes_recv < 1)
    {
      printf("Connection closed by client\n");
      close(sock_client);
      return NULL;
    }
    // parse buffer
    parse_buffer(mess, username, password);
    int response = login(username, password);

    if (response != SUCCESS && response == ERR_PASSWORD_INCORRECT)
    {
      int count = times(head, username);
      if (count == -1)
      {
        add_entry(&head, username);
        count = 1;
      }
      if (count >= MAX_PASSWORD_WRONG)
      {
        change_status(username, BLOCKED);
        reset_count(head, username);
        response = ERR_PASSWORD_INCORRECT_OVER;
      }
    }

    sprintf(mess, "%d", response);
    send(sock_client, mess, strlen(mess), 0);

    if (response == SUCCESS)
    {
      is_login = 1;
      reset_count(head, username);
    }
    else
    {
      plus_count(head, username);
    }
  }

  free_entry(head);
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
