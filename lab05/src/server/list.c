#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "list.h"

UserNode *list;

void push(User *user)
{
  User *result = search_by_username(user->username);
  if (result != NULL)
    return;

  UserNode *new_user = (UserNode *)mmap(NULL, sizeof(UserNode), PROT_READ | PROT_WRITE,
                                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  new_user->user = user;
  new_user->next = 0;

  if (list == NULL)
  {
    list = new_user;
    return;
  }

  UserNode *tmp = list;

  while (tmp->next)
  {
    tmp = tmp->next;
  }

  tmp->next = new_user;
}

void print_list()
{
  if (list == NULL)
    return;
  UserNode *tmp = list;
  do
  {
    printf("%s %s %d\n", tmp->user->username, tmp->user->password, tmp->user->status);
    tmp = tmp->next;
  } while (tmp != NULL);
}

User *search_by_username(const char *username)
{
  if (list == NULL)
  {
    return NULL;
  }

  UserNode *temp = list;

  do
  {
    if (strcmp(username, temp->user->username) == 0)
      return temp->user;
    temp = temp->next;
  } while (temp != NULL);

  return NULL;
}

void free_data()
{
  if (list == NULL)
    return;

  UserNode *tmp = list, *tmp2;
  while (tmp->next != NULL)
  {
    tmp2 = tmp;
    free(tmp->user->homepage);
    free(tmp->user->username);
    free(tmp->user->password);
    free(tmp->user);
    tmp = tmp->next;
    free(tmp2);
  }
}
