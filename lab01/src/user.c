#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "user.h"
#include "file.h"
#include "list.h"
#include "status.h"

User *currentUser = NULL;

User *create_new_user(const char *username, const char *password, int status)
{
  User *user = (User *)malloc(sizeof(User));
  if (user == NULL)
  {
    return NULL;
  }
  strcpy(user->username, username);
  strcpy(user->password, password);
  user->status = status;

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
      if (status == IDLE)
        return ERR_USER_NOT_ACTIVATE;
      else if (status == BLOCKED)
        return ERR_USER_BLOCKED;
      else
        return ERROR;
    }
    currentUser = result;
    return SUCCESS;
  }
  else
  {
    // TODO: return code password incorrect
    return ERR_PASSWORD_INCORRECT;
  }
}

int sign_out(const char *username)
{
  User *result = search_by_username(username);
  if (result == NULL)
  {
    // TODO: return code user not found
    return ERR_USER_NOT_FOUND;
  }

  if (currentUser == NULL || strcmp(currentUser->username, username) != 0)
  {
    // TODO: return code yet login
    return ERR_UNAUTHORIZED;
  }
  currentUser = NULL;

  // TODO: return code success
  return SUCCESS;
}

int change_password(const char *username, const char *old_password, const char *new_password)
{
  if (currentUser == NULL || strcmp(currentUser->username, username) != 0)
  {
    // TODO: return code yet login
    return ERR_UNAUTHORIZED;
  }

  if (strcmp(currentUser->password, old_password) != 0)
  {
    // TODO: return code password incorrect
    return ERR_OLD_PASSWORD_INCORRECT;
  }
  strcpy(currentUser->password, new_password);
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
