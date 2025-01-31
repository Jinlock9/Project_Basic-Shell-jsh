#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "jsh.h"
#include "utils.h"

void msg(const char *text) {
    printf("%s", text);
    fflush(stdout);
}

void errmsg(const char *error, const char *message) {
    printf("%s%s\n", error, message);
    fflush(stdout);
}

void bgmsg(int status, const char *command, int process_number) {
    if (status == RUN) {
        printf("[%d] running %s\n", process_number, command);
        fflush(stdout);
    } else if (status == DONE) {
        printf("[%d] done %s\n", process_number, command);
        fflush(stdout);
    } else {
        printf("[%d] %s\n", process_number, command);
        fflush(stdout);
    }
}

void sigint_handler_parent(int sig) {
    if (sig == SIGINT) {
        msg("\njsh $ ");
    }
}

void sigint_handler_parent_quit(int sig) {
    if (sig == SIGINT) {
        msg("\n");
    }
}

void sigint_handler_child(int sig) {
    if (sig == SIGINT) {
        exit(0);
    }
}

void free_loop(char **argv, char *f_in, char *f_out, int *pipes) {
    if (argv) {
        for (int i = 0; i < MAX_LINE; i++) {
            if (argv[i]) {
                free(argv[i]);
            }
        }
        free((void *)argv);
    }
    if (f_in) {
        free((void *)f_in);
    }
    if (f_out) {
        free((void *)f_out);
    }
    if (pipes) {
        free((void *)pipes);
    }
}

void free_program(char *dir_current, char *dir_before, char **bg_command, int bg_count) {
    free((void *)dir_current);
    if(dir_before) {
        free((void *)dir_before);
    }
    if (bg_command) {
        for(int i = 0; i < bg_count; i++) {
            free((void *)bg_command[i]);
        }
        free((void *)bg_command);
    }
    
}

void free_parse(char *input, int *not_string) {
    if (input) {
        free(input);
    }
    if (not_string) {
        free(not_string);
    }
}

void str_copy(char *str1, const char *str2) {
    size_t length = strlen(str2);
    for (size_t i = 0; i < length; i++) {
        str1[i] = str2[i];
    }
    str1[length] = '\0';
}

void str_init(char *str, int length) {
    for (int i = 0; i < length; i++) {
        str[i] = '\0';
    }
}
