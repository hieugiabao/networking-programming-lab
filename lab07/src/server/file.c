
#include "file.h"
#include "user.h"
#include "list.h"
#include "status.h"
#include "utils.h"

FILE *input_stream;
char filepath[100];

extern UserNode *list;

int open_input_stream(const char *file_name)
{
  strcpy(filepath, file_name);
  input_stream = fopen(file_name, "r");
  if (input_stream == NULL)
    return IO_ERROR;

  return IO_SUCCESS;
}

void close_input_stream()
{
  fclose(input_stream);
}

void get_list_users_from_file()
{
  if (list != NULL)
  {
    printf("%s\n", set_color(YELLOW, "WARNING: list of user already exists"));
    return;
  }

  char buffer[256];
  while (1)
  {
    char *read_line = fgets(buffer, 256, input_stream);
    if (read_line == NULL)
      break;
    // remove \n from string of buffer
    if (read_line[strlen(read_line) - 1] == '\n')
      read_line[strlen(read_line) - 1] = '\0';
    char **words = split(buffer, ' ');
    int i = 0;
    for (; *(words + i); i++)
      ;

    if (i != 3) // must be username password status
    {
      // TODO: raise error not correct format
      fprintf(stderr, "ERROR: file format incorrect: '<username> <password> <status> <homepage>'\n");
      exit(1);
    }

    User *user = create_new_user(words[0], words[1], strtol(words[2], NULL, 10));
    if (user == NULL)
    {
      // TODO: raise error malloc failed
      error(ERR_MEMORY_FAILED);
      exit(1);
    }
    push(user);
  }
}

void update_list_users_to_file()
{
  if (!filepath)
  {
    fprintf(stderr, "No filename specific");
    return;
  }
  input_stream = freopen(filepath, "w+", input_stream);

  UserNode *tmp = list;
  char user[256];

  while (tmp)
  {
    sprintf(user, "%s %s %d\n", tmp->user->username, tmp->user->password, tmp->user->status);
    size_t writed_bytes = fwrite(user, sizeof(char), strlen(user), input_stream);
    if (writed_bytes != strlen(user))
    {
      fprintf(stderr, "Error when update user data");
      exit(1);
    }
    tmp = tmp->next;
  }

  input_stream = freopen(filepath, "r", input_stream);
}
