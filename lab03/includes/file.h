
#ifndef __FILE_H__
#define __FILE_H__

#define IO_ERROR 0
#define IO_SUCCESS 1

#define GREY "\x1b[37;20m"
#define DIM "\x1b[2;20m"
#define GREEN "\x1b[32;20m"
#define MEGENTA "\x1b[35;20m"
#define CYAN "\x1b[36;20m"
#define YELLOW "\x1b[33;20m"
#define RED "\x1b[31;20m"
#define BOLD_RED "\x1b[31;1m"
#define RESET "\x1b[0m"

int open_input_stream(const char *file_name);
void close_input_stream();
char *trim(char *string);
void get_list_users_from_file();
void update_list_users_to_file();
char *set_color(const char *color, const char *msg, ...);
int get_password(char *password, size_t size);

#endif
