
#ifndef __LIST_H__
#define __LIST_H__

#include "user.h"

struct _UserNode
{
  struct _User *user;
  struct _UserNode *next;
};

typedef struct _UserNode UserNode;

void push(struct _User *data);
struct _User *pop();
int is_empty();
void print_list();
User *search_by_username(const char *username);

void free_data();

#endif
