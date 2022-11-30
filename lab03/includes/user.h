
#ifndef __USER_H__
#define __USER_H__

#define MAX_USERNAME_LEN 20
#define MAX_PASSWORD_LEN 20
#define MAX_HOMEPAGE_LEN 100

enum Status
{
  ACTIVATE = 1,
  BLOCKED = 0,
  IDLE = 2,
};

struct _User
{
  char *username;
  char *password;
  char *homepage;
  enum Status status;
  int is_login;
};

typedef struct _User User;

User *create_new_user(const char *username, const char *password, int status, char *homepage);
int register_new_user(User *user);
int login(const char *username, const char *password);
int sign_out(const char *username);
int change_password(const char *username, const char *old_password, const char *new_password);
int change_status(const char *username, enum Status new_status);
int valid_user(const char *username, const char *password);
int get_status(const char *username);

#endif
