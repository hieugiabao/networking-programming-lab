#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "user.h"
#include "file.h"
#include "list.h"
#include "status.h"

User *create_new_user(const char *username, const char *password, int status, char *homepage)
{
  User *user = (User *)mmap(NULL, sizeof(User), PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (user == NULL)
  {
    return NULL;
  }
  user->homepage = mmap(NULL, strlen(homepage) + 1, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  user->username = mmap(NULL, strlen(username) + 1, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  user->password = mmap(NULL, strlen(password) + 1, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  if (user->homepage == NULL || user->username == NULL || user->password == NULL)
  {
    free(user->homepage);
    free(user->username);
    free(user->password);
    free(user);
    return NULL;
  }
  strcpy(user->username, username);
  strcpy(user->password, password);
  strcpy(user->homepage, homepage);
  user->status = status;
  user->is_login = 0;
  user->wrong_password_times = 0;

  return user;
}

int register_new_user(User *user)
{
  User *result = search_by_username(user->username);
  if (result != NULL)
  {
    // TODO: return error code username already exist
    return ERR_USERNAME_EXIST;
  }
  push(user);
  update_list_users_to_file();
  // TODO: return code user created
  return CREATED;
}

int login(const char *username, const char *password)
{
  User *result = search_by_username(username);
  if (result == NULL)
  {
    // TODO: return code user not found
    return ERR_USER_NOT_FOUND;
  }

  if (strcmp(password, result->password) == 0)
  {
    // TODO: return code login success
    int status = get_status(username);
    if (status != ACTIVATE)
    {
      result->wrong_password_times = 0;
      if (status == IDLE)
        return ERR_USER_NOT_ACTIVATE;
      else if (status == BLOCKED)
        return ERR_USER_BLOCKED;
      else
        return ERROR;
    }
    result->is_login = 1;
    return SUCCESS;
  }
  else
  {
    // TODO: return code password incorrect
    result->wrong_password_times++;
    return ERR_PASSWORD_INCORRECT;
  }
}

int logout(const char *username)
{
  User *result = search_by_username(username);
  if (result == NULL)
  {
    // TODO: return code user not found
    return ERR_USER_NOT_FOUND;
  }

  if (result->is_login == 0)
  {
    // TODO: return code yet login
    return ERR_UNAUTHORIZED;
  }
  result->is_login = 0;

  // TODO: return code success
  return SUCCESS;
}

int change_password(const char *username, const char *old_password, const char *new_password)
{
  User *user = search_by_username(username);
  if (!user || user->is_login == 0)
  {
    // TODO: return code yet login
    return ERR_UNAUTHORIZED;
  }

  if (strcmp(user->password, old_password) != 0)
  {
    // TODO: return code password incorrect
    return ERR_OLD_PASSWORD_INCORRECT;
  }
  strcpy(user->password, new_password);
  update_list_users_to_file();
  // TODO: return code success
  return SUCCESS;
}

int change_status(const char *username, enum Status new_status)
{
  User *user = search_by_username(username);
  if (user == NULL)
  {
    // TODO: return code user not found
    return ERR_USER_NOT_FOUND;
  }
  user->status = new_status;
  update_list_users_to_file();
  // TODO: return code success
  return SUCCESS;
}

int valid_user(const char *username, const char *password)
{
  User *result = search_by_username(username);
  if (result == NULL)
  {
    // TODO: return code user not found
    return ERR_USER_NOT_FOUND;
  }

  if (strcmp(password, result->password) == 0)
  {
    // TODO: return code login success
    return SUCCESS;
  }
  else
  {
    // TODO: return code password incorrect
    return ERR_PASSWORD_INCORRECT;
  }
}

int get_status(const char *username)
{
  User *user = search_by_username(username);
  if (!user)
    return ERR_USER_NOT_FOUND;
  return user->status;
}
