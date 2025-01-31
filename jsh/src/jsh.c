#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "jsh.h"
#include "utils.h"
#include "func.h"

int main(void) {
    char *dir_home = getenv("HOME");  // ~
    int dir_before_update = 0;
    char *dir_before = (char *)malloc(sizeof(char) * MAX_PATH);
    if (dir_before == NULL) {
        exit(EXIT_FAILURE);
    }
    str_init(dir_before, MAX_PATH);
    char *dir_current = (char *)malloc(sizeof(char) * MAX_PATH);
    if (dir_current == NULL) {
        exit(EXIT_FAILURE);
    }
    str_init(dir_current, MAX_PATH);
    char dir_temp_init[MAX_PATH];
    getcwd(dir_temp_init, sizeof(dir_temp_init));
    str_copy(dir_current, dir_temp_init);

    int bg_count = 0;
    int *job_status = (int *)malloc(sizeof(int) *MAX_JOBS);
    if (job_status == NULL) {
        exit(EXIT_FAILURE);
    }
    char **bg_command = (char **)malloc(sizeof(char *) * MAX_JOBS);
    if (bg_command == NULL) {
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MAX_JOBS; i++) {
        bg_command[i] = NULL;
    }

    while (1) {
        signal(SIGINT, &sigint_handler_parent);

        msg(PROMPT);

        char *input = (char *)malloc(sizeof(char) * MAX_LINE);
        if (input == NULL) {
            exit(EXIT_FAILURE);
        }
        str_init(input, MAX_LINE);
        int *not_string = (int *)malloc(sizeof(int) * MAX_LINE);
        if (not_string == NULL) {
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < MAX_LINE; i++) {
            not_string[i] = 0;
        }

        int syntax_error = parser(input, not_string, dir_current, dir_before, bg_command, bg_count);

        if (!syntax_error) {
            signal(SIGINT, &sigint_handler_parent_quit);

            char **argv = (char **)malloc(sizeof(char *) * MAX_LINE);
            if (argv == NULL) {
                exit(EXIT_FAILURE);
            }
            for (int i = 0; i < MAX_LINE; i++) {
                argv[i] = NULL;
            }
            char *f_in = (char *)malloc(sizeof(char) * MAX_LINE);
            if (f_in == NULL) {
                exit(EXIT_FAILURE);
            }
            str_copy(f_in, "");
            char *f_out = (char *)malloc(sizeof(char) * MAX_LINE);
            if (f_out == NULL) {
                exit(EXIT_FAILURE);
            }
            str_copy(f_out, "");
            int is_append = 0;

            int pipecnt = 0;
            int *pipes = (int *)malloc(sizeof(int) * MAX_PIPE);
            if (pipes == NULL) {
                exit(EXIT_FAILURE);
            }
            pipes[0] = 0;

            size_t inpl = strlen(input);
            int is_background = 0;
            if (input[inpl - 2] == '&') {
                is_background = 1;
                bg_command[bg_count] = (char *)malloc(sizeof(char) * MAX_LINE);
                if (bg_command[bg_count] == NULL) {
                    exit(EXIT_FAILURE);
                }
                str_init(bg_command[bg_count], MAX_LINE);
                str_copy(bg_command[bg_count], input);
                bg_command[bg_count][strlen(bg_command[bg_count]) - 1] = '\0';
            }
            if (inpl >0 && input[inpl - 1] == '\n') {
                input[inpl - 1] = '\0';
            }

            int io_error = tokenizer(input, not_string, argv, &is_append, f_in, f_out, pipes, &pipecnt);
            free_parse(input, not_string);

            if (!io_error) {
                dir_before_update = executer(argv, f_in, f_out, is_append, pipes, pipecnt, dir_home, dir_current, dir_before, dir_before_update, bg_command, &bg_count, job_status, is_background);
            } else {
                free_loop(argv, f_in, f_out, pipes);
            }
        } else {
            free_parse(input, not_string);
        }
    }
    free_program(dir_current, dir_before, bg_command, bg_count);
    return 0;
}
