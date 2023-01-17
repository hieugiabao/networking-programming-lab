#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

#if !defined(_WIN32)
#include <termios.h>
#include <unistd.h>
#endif

#define GREY "\x1b[37;20m"
#define DIM "\x1b[2;20m"
#define GREEN "\x1b[32;20m"
#define MEGENTA "\x1b[35;20m"
#define CYAN "\x1b[36;20m"
#define YELLOW "\x1b[33;20m"
#define RED "\x1b[31;20m"
#define BOLD_RED "\x1b[31;1m"
#define RESET "\x1b[0m"

char *trim(char *string);
char *set_color(const char *color, const char *msg, ...);
int get_password(char *password, size_t size);
int is_valid_password(const char *password);
void parse_buffer(const char *buffer, char *part1, char *part2);
char **split(char *string, const char delimiter);

#endif
