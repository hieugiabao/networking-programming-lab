#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "list.h"
#include "status.h"

#define FILE_PATH "account.txt"
#define CODE_ACTIVATE "20194561"

extern User *currentUser;

void show_menu();
void register_user();
void activate();
void signin();
void search();
void change_pass();
void signout();

int main(int argc, char const *argv[])
{
  if (open_input_stream(FILE_PATH) == IO_ERROR)
  {
    fprintf(stderr, "Error occured: Can't open file '%s' to load user infomation", FILE_PATH);
  }
  get_list_users_from_file();
  show_menu();
  close_input_stream();
  free_data();
  return 0;
}

void show_menu()
{
  char choice[20], *tmp;
  int loop = 1;

  while (loop != 0)
  {
    printf(
        "USER MANAGEMENT PROGRAM\n"
        "-----------------------------------\n"
        "1. Register\n"
        "2. Activate\n"
        "3. Sign in\n"
        "4. Search\n"
        "5. Change password\n"
        "6. Sign out\n"
        "Your choice (1-6, other to quit): ");
    scanf("%s", choice);
    while (getchar() != '\n')
      ;
    tmp = choice;
    while (tmp && *tmp != '\0')
    {
      if (!isdigit(*tmp))
      {
        return;
      }
      tmp++;
    }
    switch (atoi(choice))
    {
    case 1:
      register_user();
      break;
    case 2:
      activate();
      break;
    case 3:
      signin();
      break;
    case 4:
      search();
      break;
    case 5:
      change_pass();
      break;
    case 6:
      signout();
      break;
    default:
      loop = 0;
      break;
    }
    printf("\n");
  }
}

void register_user()
{
  char username[MAX_USERNAME_LEN + 1];
  char password[MAX_PASSWORD_LEN + 1];
  char homepage[MAX_HOMEPAGE_LEN + 1];
  printf("Username: ");
  scanf("%s", username);
  while (getchar() != '\n')
    ;
  printf("Password: ");
  get_password(password, MAX_PASSWORD_LEN + 1);
  printf("Homepage: ");
  scanf("%s", homepage);
  User *user = create_new_user(trim(username), trim(password), 2, homepage);
  if (!user)
  {
    error(ERR_MEMORY_FAILED);
    exit(1);
  }
  int response = register_new_user(user);

  if (response == CREATED)
  {
    printf("%s\n", set_color(GREEN, "Successful registration. Activation required."));
  }
  else
  {
    error(response);
  }
}

void activate()
{
  char username[MAX_USERNAME_LEN + 1];
  char password[MAX_PASSWORD_LEN + 1];
  char code[1024];
  int count = 1;

  printf("Username: ");
  scanf("%s", username);
  while (getchar() != '\n')
    ;
  printf("Password: ");
  get_password(password, MAX_PASSWORD_LEN + 1);
  printf("Code: ");
  get_password(code, 1025);

  int response = valid_user(trim(username), trim(password));
  if (response != SUCCESS)
  {
    error(response);
    return;
  }

  if (get_status(username) == ACTIVATE)
  {
    printf("%s\n", set_color(GREEN, "Your account is already activated."));
    return;
  }

  while (1)
  {
    if (count != 1)
    {
      printf("Code: ");
      get_password(code, 1025);
    }

    if (strcmp(code, CODE_ACTIVATE) != 0)
    {
      if (count == 4)
      {
        error(ERR_CODE_ACTIVATE_INCORRECT_OVER);
        change_status(username, BLOCKED);
        return;
      }
      error(ERR_CODE_ACTIVATE_INCORRECT);
      count++;
    }
    else
    {
      response = change_status(username, ACTIVATE);
      printf("%s\n", set_color(GREEN, "Account is activated."));
      if (response != SUCCESS)
      {
        error(response);
      }
      return;
    }
  }
}

void signin()
{
  char username[MAX_USERNAME_LEN + 1];
  char password[MAX_PASSWORD_LEN + 1];
  int count = 1;

  printf("Username: ");
  scanf("%s", username);
  while (getchar() != '\n')
    ;
  printf("Password: ");
  get_password(password, MAX_PASSWORD_LEN + 1);

  while (1)
  {
    if (count != 1)
    {
      printf("Password: ");
      get_password(password, MAX_PASSWORD_LEN + 1);
    }
    int response = login(trim(username), trim(password));
    if (response == SUCCESS)
    {
      printf("%s\n", set_color(GREEN, "Hello %s", currentUser->username));
      break;
    }
    else
    {
      if (response == ERR_PASSWORD_INCORRECT)
      {
        if (count == 3)
        {
          error(ERR_PASSWORD_INCORRECT_OVER);
          change_status(username, BLOCKED);
          break;
        }
        error(response);
        count++;
        continue;
      }
      else
      {
        error(response);
        break;
      }
    }
  }
}

void search()
{
  char username[MAX_USERNAME_LEN + 1];
  printf("Username: ");
  scanf("%s", username);
  while (getchar() != '\n')
    ;

  int status = get_status(username);
  if (status == ERR_USER_NOT_FOUND)
  {
    error(ERR_USER_NOT_FOUND);
  }
  else
  {
    switch (status)
    {
    case ACTIVATE:
      printf("%s\n", set_color(GREEN, "Account is active."));
      break;
    case BLOCKED:
      printf("%s\n", set_color(GREEN, "Account is blocked."));
      break;
    case IDLE:
      printf("%s\n", set_color(GREEN, "Account not active."));
      break;
    default:
      break;
    }
  }
}

void change_pass()
{
  char username[MAX_USERNAME_LEN + 1];
  char old_password[MAX_PASSWORD_LEN + 1];
  char new_password[MAX_PASSWORD_LEN + 1];
  printf("Username: ");
  scanf("%s", username);
  while (getchar() != '\n')
    ;
  printf("Old password: ");
  get_password(old_password, MAX_PASSWORD_LEN + 1);
  printf("New password: ");
  get_password(new_password, MAX_PASSWORD_LEN + 1);

  int response = change_password(username, old_password, new_password);
  if (response == SUCCESS)
  {
    printf("%s\n", set_color(GREEN, "Password is changed."));
  }
  else
  {
    error(response);
  }
}

void signout()
{
  char username[MAX_USERNAME_LEN + 1];
  printf("Username: ");
  scanf("%s", username);
  while (getchar() != '\n')
    ;
  int response = sign_out(username);
  if (response != SUCCESS)
    error(response);
  else
    printf("%s\n", set_color(GREEN, "Bye %s", username));
}
