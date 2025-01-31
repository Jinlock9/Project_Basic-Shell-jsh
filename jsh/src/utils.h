#ifndef UTILS_H
#define UTILS_H

void msg(const char *text);
void errmsg(const char *error, const char *message);
void bgmsg(int status, const char *command, int process_number);

void sigint_handler_parent(int sig);
void sigint_handler_parent_quit(int sig);
void sigint_handler_child(int sig);

void free_loop(char **argv, char *f_in, char *f_out, int *pipes);
void free_program(char *dir_current, char *dir_before, char **bg_command, int bg_count);
void free_parse(char *input, int *not_string);

void str_copy(char *str1, const char *str2);
void str_init(char *str, int length);

#endif
