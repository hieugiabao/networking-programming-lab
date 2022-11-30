#include "utils.h"

char *ltrim(char *string);
char *rtrim(char *string);

char *trim(char *string)
{
  return rtrim(ltrim(string));
}

char *ltrim(char *s)
{
  while (isspace(*s))
    s++;
  return s;
}

char *rtrim(char *s)
{
  char *back = s + strlen(s);
  while (isspace(*--back))
    ;
  *(back + 1) = '\0';
  return s;
}

char **split(char *string, const char delimiter)
{
  char **result = 0;
  size_t count = 0;
  char *s = string;
  char *last_comma = 0;
  char delim[2];
  delim[0] = delimiter;
  delim[1] = 0;

  while (*s)
  {
    if (delimiter == *s)
    {
      count++;
      last_comma = s;
    }
    s++;
  }

  count += last_comma < (string + strlen(string) - 1);
  count++;

  result = malloc(sizeof(char *) * count);
  if (result)
  {
    size_t idx = 0;
    char *token = strtok(string, delim);
    while (token)
    {
      assert(idx < count);
      *(result + idx++) = strdup(token);
      token = strtok(0, delim);
    }
    *(result + idx) = 0;
  }

  return result;
}

char *set_color(const char *color, const char *msg, ...)
{
#define BUFFER_SIZE 2048
  char *buffer = (char *)malloc(BUFFER_SIZE);
  char *result = (char *)malloc(BUFFER_SIZE);
  sprintf(buffer, "%s%s%s", color, msg, RESET);

  va_list args;
  va_start(args, buffer);
  vsprintf(result, buffer, args);
  va_end(args);

  free(buffer);
  return result;
}

int get_password(char *password, size_t size)
{
  size_t idx = 0;
  if (password == NULL)
  {
    void *tmp = realloc(password, size * sizeof(char));
    if (!tmp)
    {
      return -1;
    }
    memset(tmp, 0, size);
    password = (char *)tmp;
  }

#if defined(_WIN32)
  char ch;
  while ((ch = _getch()) != 13 && idx < size)
  {
    password[idx++] = ch;
    printf("*");
  }
  password[idx] = '\0';
#else
  struct termios oldt, newt;
  int c;

  /*saving the old settings of STDIN_FILENO and copy settings for resetting*/
  if (tcgetattr(STDIN_FILENO, &oldt))
  {
    fprintf(stderr, "%s() error: tcgetattr() failed.\n", __func__);
    return -1;
  }
  memcpy(&newt, &oldt, sizeof(struct termios));

  /*setting the approriate bit in the termios struct*/
  newt.c_lflag &= ~(ICANON | ECHO);
  newt.c_cc[VTIME] = 0;
  newt.c_cc[VMIN] = 1;

  /*setting the new bits*/
  if (tcsetattr(STDIN_FILENO, TCSANOW, &newt))
  {
    fprintf(stderr, "%s() error: tcsetattr failed.\n", __func__);
    return -1;
  }

  /*reading the password from the console*/
  while (
      ((c = fgetc(stdin)) != '\n' && c != EOF && idx < size - 1) || (idx == size - 1 && c == 127))
  {
    if (c != 127)
    {
      fputc('*', stdout);
      password[idx++] = c;
    }
    else if (idx > 0)
    { /* handle backspace (del)   */
      fputc(0x8, stdout);
      fputc(' ', stdout);
      fputc(0x8, stdout);
      password[--idx] = 0;
    }
  }
  password[idx] = '\0';

  /*resetting our old STDIN_FILENO*/
  if (tcsetattr(STDIN_FILENO, TCSANOW, &oldt))
  {
    fprintf(stderr, "%s() error: tcsetattr failed.\n", __func__);
    return -1;
  }

  if (idx == size - 1 && c != '\n') /* warn if pw truncated */
    fprintf(stderr, " (%s() warning: truncated at %zu chars.)\n",
            __func__, size - 1);
#endif

  printf("\n");
  return idx;
}

int is_valid_password(const char *password)
{
  if (password == NULL)
    return 0;
  // password contains charater is a letter or a number
  for (int i = 0; i < strlen(password); i++)
  {
    if (!isalnum(password[i]))
      return 0;
  }
  return 1;
}

void parse_buffer(const char *buffer, char *username, char *password)
{
  char *strpt = (char *)buffer;
  int i = 0;
  memcpy(username, strpt, strlen(strpt));
  username[strlen(strpt)] = '\0';

  strpt += strlen(strpt) + 1;
  memcpy(password, strpt, strlen(strpt));
  password[strlen(strpt)] = '\0';
}
