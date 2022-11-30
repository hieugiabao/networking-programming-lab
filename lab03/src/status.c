#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "status.h"
#include "file.h"

void error(ErrorCode code)
{
  char *msg;
  switch (code)
  {
  case ERR_UNAUTHORIZED:
    msg = "Acount is not sign in";
    break;
  case ERR_USERNAME_EXIST:
    msg = "Account existed";
    break;
  case ERR_USER_NOT_FOUND:
    msg = "Cannot find account";
    break;
  case ERR_PASSWORD_INCORRECT:
    msg = "Password is incorrect";
    break;
  case ERR_CODE_ACTIVATE_INCORRECT:
    msg = "Code active incorrect. Account is not activated";
    break;
  case ERR_CODE_ACTIVATE_INCORRECT_OVER:
    msg = "Activation code is incorrect. Account is blocked";
    break;
  case ERR_PASSWORD_INCORRECT_OVER:
    msg = "Password is incorrect. Account is blocked";
    break;
  case ERR_USER_NOT_ACTIVATE:
    msg = "Account not activated. Please activate your account";
    break;
  case ERR_USER_BLOCKED:
    msg = "Account is blocking. Please activate your account";
    break;
  case ERR_MEMORY_FAILED:
    msg = "Memory allocation failed";
    break;
  case ERR_OLD_PASSWORD_INCORRECT:
    msg = "Current password is incorrect. Please try again";
  case ERR_IPADDR_NOT_FOUND:
    msg = "Cannot find IP address";
    break;
  case ERR_DOMAIN_NOT_FOUND:
    msg = "Cannot find domain address";
    break;
  default:
    msg = "Error undefined";
    break;
  }

  fprintf(stderr, "%s\n", set_color(RED, msg));
}
