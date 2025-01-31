#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>      
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stddef.h>

#include "jsh.h"
#include "func.h"
#include "utils.h"

int parser(char* input, int* not_string, char *dir_current, char *dir_before, char **bg_command, int bg_count) {
    int syntax_error = 0;
    int is_first_input = 1;
    int quote_opened = 0;
    int double_quotes_opened = 0;
    int single_quotes_opened = 0;
    int command_not_completed = 0;
    int string_count = 0;
    size_t inpl = 0;
    while (!syntax_error && (is_first_input || quote_opened || command_not_completed)) {
        char input_buffer[MAX_LINE] = "";
        if (fgets(input_buffer, MAX_LINE, stdin) != NULL) {
            if(input_buffer[0] == '\n') {
                if (quote_opened || command_not_completed) {
                    msg(ENTER);
                } else {
                    msg(PROMPT);
                }
            } else {
                is_first_input = 0;
                size_t inbl = strlen(input_buffer);
                for (size_t i = 0; i < inbl; i++) {
                    if (input_buffer[i] == '\"' && !single_quotes_opened) {
                        if (double_quotes_opened) {
                            double_quotes_opened = 0; // double quote closed
                        } else {
                            double_quotes_opened = 1; // double quote opend
                        }
                    } else if (input_buffer[i] == '\'' && !double_quotes_opened) {
                        if (single_quotes_opened) {
                            single_quotes_opened = 0; // single quote closed
                        } else {
                            single_quotes_opened = 1; // single quote opend
                        }
                    } else if (double_quotes_opened || single_quotes_opened) {
                        if (input_buffer[i] == ' ') {
                            string_count++;
                        }
                        if (command_not_completed) {
                            command_not_completed = 0;
                        }
                        input[inpl++] = input_buffer[i];
                    } else {
                        if (input_buffer[i] == '>' || input_buffer[i] == '<' || input_buffer[i] == '|') {
                            if (input_buffer[i] == '>' && i > 1 && input[inpl - 2] == '>') {
                                errmsg("syntax error near unexpected token ", "`>'");
                                syntax_error = 1;
                            } else if (input_buffer[i] == '<' && i > 1 && (input[inpl - 2] == '>' || input[inpl - 2] == '<')) {
                                errmsg("syntax error near unexpected token ", "`<'");
                                syntax_error = 1;
                            } else if (input_buffer[i] == '|' && i > 1 && (input[inpl - 2] == '>' || input[inpl - 2] == '<')) {
                                errmsg("syntax error near unexpected token ", "`|'");
                                syntax_error = 1;
                            }
                            if (syntax_error) {
                                break;
                            }
                            if (i > 0 && input_buffer[i - 1] != ' ' && input[inpl - 1] != ' ') {
                                input[inpl++] = ' ';
                                string_count++;
                            }
                            if (i < inbl && input_buffer[i] == '>' && input_buffer[i + 1] == '>') {
                                input[inpl++] = input_buffer[i++];
                                input[inpl++] = input_buffer[i];
                            } else {
                                input[inpl++] = input_buffer[i];
                            }
                            not_string[string_count] = 1;
                            command_not_completed = 1;
                            if (i < inbl && input_buffer[i + 1] != ' ') {
                                input[inpl++] = ' ';
                                string_count++;
                            }
                        } else {
                            if (input_buffer[i] == ' ') {
                                string_count++;
                            }
                            if (command_not_completed && input_buffer[i] != '\n') {
                                command_not_completed = 0;
                            }
                            int cont = 0;
                            if (command_not_completed && input_buffer[i] == '\n') {
                                cont = 1;
                            } else {
                                input[inpl++] = input_buffer[i];
                            }
                            if (cont) {
                                continue;
                            }
                        }
                    }
                }
                if (single_quotes_opened || double_quotes_opened) {
                    quote_opened = 1;
                    msg(ENTER);
                } else if (command_not_completed && !syntax_error) {
                    msg(ENTER);
                } else {
                    quote_opened = 0;
                }   
            }
        } else if (feof(stdin)) {
            if (is_first_input) {
                msg("exit\n");
                free_parse(input, not_string);
                free_program(dir_current, dir_before, bg_command, bg_count);
                exit(0);
            } else {
                clearerr(stdin);
            }
        } 
    }
    return syntax_error;
}

int tokenizer(char *input, const int *not_string, char **argv, int *is_append, char *f_in, char *f_out, int *pipes, int *pipecnt) {
    int io_error = 0;
    int duplicated_input = 0;
    int duplicated_output = 0;

    const char *token = NULL;
    token = strtok(input, " ");
    int str_tracker = 0;
    int argc = 0;
    while (token != NULL && !io_error) {
        if (not_string[str_tracker]) {
            if (!strcmp(token, ">")) {
                if (duplicated_output) {
                    errmsg("error", ": duplicated output redirection");
                    io_error = 1;
                } else {
                    duplicated_output = 1;
                }
                if (!io_error) {
                    token = strtok(NULL, " ");
                    str_copy(f_out, token);
                    str_tracker++;
                }
            } else if (!strcmp(token, ">>")) {
                if (duplicated_output) {
                    errmsg("error", ": duplicated output redirection");
                    io_error = 1;
                } else {
                    duplicated_output = 1;
                }
                if (!io_error) {
                    token = strtok(NULL, " ");
                    str_copy(f_out, token);
                    *is_append = 1;
                    str_tracker++;
                }
            } else if (!strcmp(token, "<")) {
                if (duplicated_input) {
                    errmsg("error", ": duplicated input redirection");
                    io_error = 1;
                } else {
                    duplicated_input = 1;
                }
                if (!io_error) {
                    token = strtok(NULL, " ");
                    str_copy(f_in, token);
                    str_tracker++;
                }
            } else if (!strcmp(token, "|")) {
                if ((argc > 0 && argv[argc - 1] == NULL) || argc == 0) {
                    errmsg("error", ": missing program");
                    io_error = 1;
                    break;
                }
                if (duplicated_output) {
                    errmsg("error", ": duplicated output redirection");
                    io_error = 1;
                    break;
                }
                duplicated_input = 1;
                argv[argc++] = NULL;
                *pipecnt = *pipecnt + 1;
                (pipes)[*pipecnt] = argc;
            } 
        } else if(strcmp(token, "&") != 0) {
            argv[argc] = (char *)malloc(strlen(token) + 1);
            if (argv[argc] == NULL) {
                exit(EXIT_FAILURE);
            }
            str_copy(argv[argc], token);
            argc++;
        }
        if (!io_error) {
            token = strtok(NULL, " ");
            str_tracker++;
        }
    }
    argv[argc] = NULL;
    if (!io_error && argc == 0 && duplicated_output) {
        errmsg("error", ": missing program");
        io_error = 1;
    }

    return io_error;
}

int executer(char **argv, char *f_in, char *f_out, int is_append, int *pipes, int pipecnt, char *dir_home, char *dir_current, char *dir_before, int dir_before_update, char **bg_command, int *bg_count, int *job_status, int is_background) {
    int tmpin = dup(0);
    int tmpout = dup(1);

    int pid_status[MAX_PIPE]; 
    for (int i = 0; i < MAX_PIPE; i++) {
        pid_status[i] = -1;
    }
    
    int fdpipes[MAX_PIPE];
    for (int i = 0; i < pipecnt; i++) {
        if (pipe(fdpipes + (ptrdiff_t)(i * 2)) < 0) {
            exit(EXIT_FAILURE);
        }
    }
    int child_process = 0;
    for (int i = 0; i <= pipecnt; i++) {

        if (!strcmp(argv[pipes[i]], "exit")) {
            msg("exit\n");
            free_loop(argv, f_in, f_out, pipes);
            free_program(dir_current, dir_before, bg_command, *bg_count);
            exit(0);
        }

        if (!strcmp(argv[pipes[i]], "cd")) {
            if (argv[pipes[i] + 1] == NULL || !strcmp(argv[pipes[i] + 1], "~")) {  // cd home directory
                dir_before_update = 1;
                chdir(dir_home);
                str_copy(dir_before, dir_current);
                str_copy(dir_current, dir_home);
            } else if (!strcmp(argv[pipes[i] + 1], "-")) {
                if (dir_before_update) {
                    chdir(dir_before);
                    char dir_temp[MAX_PATH];
                    str_copy(dir_temp, dir_current);
                    str_copy(dir_current, dir_before);
                    str_copy(dir_before, dir_temp);
                } else {
                    msg("-bash: cd: OLDPWD not set\n");
                }
            } else {
                if (chdir(argv[pipes[i] + 1]) < 0) {
                    errmsg(argv[pipes[i] + 1], ": No such file or directory");
                } else {
                    dir_before_update = 1;
                    str_copy(dir_before, dir_current);
                    char dir_temp[MAX_PATH];
                    getcwd(dir_temp, sizeof(dir_temp));
                    str_copy(dir_current, dir_temp);
                }
            }
            continue;
        }
        if (!strcmp(argv[pipes[i]], "jobs")) {
            for (int j = 0; j < *bg_count; j++) {
                if (waitpid(job_status[(ptrdiff_t)(j * 2)], NULL, WNOHANG) == 0) {
                    bgmsg(RUN, bg_command[j], j + 1);
                } else {
                    bgmsg(DONE, bg_command[j], j + 1);
                }
            }
            continue;
        }

        pid_t pid = fork();
        pid_status[i] = pid;
        
        if (pid > 0 && is_background && i == 0) {
            job_status[(ptrdiff_t)(*bg_count * 2)] = pid;
            job_status[(ptrdiff_t)(*bg_count * 2) + 1] = RUN;
            *bg_count = *bg_count + 1;
            bgmsg(FALSE, bg_command[*bg_count - 1], *bg_count);
        } 

        if (pid == 0) { // CHILD
            signal(SIGINT, &sigint_handler_child);
            if (i > 0) {
                dup2(fdpipes[(ptrdiff_t)(i - 1) * 2], 0);
            }
            if (i < pipecnt) {
                dup2(fdpipes[(ptrdiff_t)(i * 2) + 1], 1);
            }
            
            if (i == 0 && strcmp(f_in, "") != 0) {
                int fdin = open(f_in, O_RDONLY);
                if (fdin < 0) {
                    errmsg(f_in, ": No such file or directory");
                    exit(EXIT_FAILURE);
                }
                dup2(fdin, 0);
                close(fdin);
            }

            if (i == pipecnt && strcmp(f_out, "") != 0) {
                if (is_append) {
                    int fdout = open(f_out, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
                    if (fdout < 0) {
                        errmsg(f_out, ": Permission denied");
                        exit(EXIT_FAILURE);
                    }
                    dup2(fdout, 1);
                    close(fdout);
                } else {
                    int fdout = open(f_out, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
                    if (fdout < 0) {
                        errmsg(f_out, ": Permission denied");
                        exit(EXIT_FAILURE);
                    }
                    dup2(fdout, 1);
                    close(fdout);
                }
            }

            for (int k = 0; k < 2 * pipecnt; k++) {
                close(fdpipes[k]);
            }

            if (!strcmp(argv[pipes[i]], "pwd")) {
                char cwd[MAX_PATH];
                getcwd(cwd, sizeof(cwd));
                msg(cwd);
                msg("\n");
                exit(0);
            }
            
            if (execvp(argv[pipes[i]], &argv[pipes[i]]) < 0) {
                errmsg(argv[pipes[i]], ": command not found");
            }
            exit(0);
        } else if (pid < 0) {
            free_loop(argv, f_in, f_out, pipes);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < 2 * pipecnt; i++) {
        close(fdpipes[i]);
    }

    if (is_background) {
        waitpid(job_status[(ptrdiff_t)(*bg_count - 1) * 2], &child_process, WNOHANG);
    } else {
        for (int i = 0; i <= pipecnt; i++) {
            if (pid_status[i] != -1) {
                waitpid(pid_status[i], NULL, WUNTRACED);
            }
        }
    }

    dup2(tmpin, STDIN_FILENO);
    dup2(tmpout, STDOUT_FILENO);
    close(tmpin);
    close(tmpout);

    free_loop(argv, f_in, f_out, pipes);
    return dir_before_update;
}
