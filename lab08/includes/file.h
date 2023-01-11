
#ifndef __FILE_H__
#define __FILE_H__

#define IO_ERROR 0
#define IO_SUCCESS 1

int open_input_stream(const char *file_name);
void close_input_stream();
void get_list_users_from_file();
void update_list_users_to_file();

#endif
