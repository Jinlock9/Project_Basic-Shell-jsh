#ifndef FUNC_H
#define FUNC_H

int parser(char* input, int* not_string, char *dir_current, char *dir_before, char **bg_command, int bg_count);
int tokenizer(char *input, const int *not_string, char **argv, int *is_append, char *f_in, char *f_out, int *pipes, int *pipecnt);
int executer(char **argv, char *f_in, char *f_out, int is_append, int *pipes, int pipecnt, char *dir_home, char *dir_current, char *dir_before, int dir_before_update, char **bg_command, int *bg_count, int *job_status, int is_background);

#endif
