#include "jsh.h"

int not_ctrl_c;

void msg(const char *message) {
    printf("%s", message);
    fflush(stdout);
}

void errmsg(const char *error, const char *message) {
    printf("%s%s\n", error, message);
    fflush(stdout);
}

void bgmsg(const int status, const int process_number, const char *command) {
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
        not_ctrl_c = 0;
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

int main(void) {
    char *prompt = "jsh $ ";
    char *enter = "> ";
    not_ctrl_c = 1;

    /* Initialize Background Process
     * @bg_count: the number of background processes
     */
    int bg_count = 0;
    int job_status[MAX_JOBS];
    char *bg_command[MAX_JOBS];

    /* Initialize Directory Tracking Process */
    char *dir_home = getenv("HOME");  // ~
    char *dir_before = (char *)malloc(sizeof(char) * MAX_PATH);
    if (dir_before == NULL) return 0;
    memset(dir_before, 0, MAX_PATH);
    char *dir_current = (char *)malloc(sizeof(char) * MAX_PATH);
    if (dir_current == NULL) {
        free(dir_before);
        return 0;
    }
    memset(dir_current, 0, MAX_PATH);
    char dir_init[MAX_PATH];
    getcwd(dir_init, sizeof(dir_init));
    strcpy(dir_current, dir_init);

    while(1) {
        signal(SIGINT, &sigint_handler_parent);
        msg(prompt);

        /* Processing Input
         * @argv: array for commands
         * @input_processed: processed input for parsing
         * @input: raw input
         */
        char **argv = (char **)malloc(sizeof(char *) * MAX_LINE);
        if (argv == NULL) return 0;
        for (int i = 0; i < MAX_LINE; i++) {
            argv[i] = NULL;
        }
        char *file_out = NULL;
        char *file_in = NULL;
        char *input_processed = (char *)malloc(sizeof(char) * MAX_LINE);
        if (input_processed == NULL) return 0;
        memset(input_processed, 0, MAX_LINE);
        int input_processed_count = 0;
        int is_first_input = 1;
        int input_not_empty = 1;

        /* Initialize Quote Handler
         * @not_string: location for >, <, >>, | that is not string in @input_processed
         */
        int quote_opened = 0;
        int double_quotes_opened = 0;
        int single_quotes_opened = 0;
        int command_not_completed = 0;
        int not_string[MAX_LINE] = {0};
        int string_count = 0;
        int syntax_error = 0;
        while (!syntax_error && (is_first_input || quote_opened || command_not_completed)) {
            char *input = (char *)malloc(sizeof(char) * MAX_LINE);
            if (input == NULL) return 0;
            memset(input, 0, MAX_LINE);
            if (fgets(input, MAX_LINE, stdin) == NULL) {
                /* Handling Ctrl-D */
                msg("exit\n");
                free(input);
                free(input_processed);
                free(argv);
                free(dir_current);
                free(dir_before);
                for(int i = 0; i < bg_count; i++) {
                    free(bg_command[i]);
                }
                exit(0);
            } else if(input[0] == '\n') {
                input_not_empty = 0;
                free(input);
                if (quote_opened || command_not_completed) {
                    msg(enter);
                }
            } else {
                input_not_empty = 1;
            }

            if (input_not_empty) {
                /* Handling Ctrl-C */
                if (!not_ctrl_c) {
                    not_ctrl_c = 1;
                    memset(input_processed, 0, MAX_LINE);
                    input_processed_count = 0;
                    quote_opened = 0;
                    command_not_completed = 0;
                    double_quotes_opened = 0;
                    single_quotes_opened = 0;
                }

                /* Process RAW Input for Parsing + Syntax Error Handling */
                size_t inl = strlen(input);
                for (size_t i = 0; i < inl; i++) {
                    if (input[i] == '\"' && !single_quotes_opened) {
                        if (double_quotes_opened) {
                            double_quotes_opened = 0; // double quote closed
                        } else {
                            double_quotes_opened = 1; // double quote opend
                        }
                    } else if (input[i] == '\'' && !double_quotes_opened) {
                        if (single_quotes_opened) {
                            single_quotes_opened = 0; // single quote closed
                        } else {
                            single_quotes_opened = 1; // single quote opend
                        }
                    } else if (double_quotes_opened || single_quotes_opened) {
                        if (input[i] == ' ') {
                            string_count++;
                        }
                        if (command_not_completed) {
                            command_not_completed = 0;
                        }
                        input_processed[input_processed_count++] = input[i];
                    } else {
                        if (input[i] == '>' || input[i] == '<' || input[i] == '|') {
                            if (input[i] == '>' && i > 1 && input_processed[input_processed_count - 2] == '>') {
                                errmsg("syntax error near unexpected token ", "`>'");
                                syntax_error = 1;
                                break;
                            } else if (input[i] == '<' && i > 1 && (input_processed[input_processed_count - 2] == '>' || input_processed[input_processed_count - 2] == '<')) {
                                errmsg("syntax error near unexpected token ", "`<'");
                                syntax_error = 1;
                                break;
                            } else if (input[i] == '|' && i > 1 && (input_processed[input_processed_count - 2] == '>' || input_processed[input_processed_count - 2] == '<')) {
                                errmsg("syntax error near unexpected token ", "`|'");
                                syntax_error = 1;
                                break;
                            }
                            if (i > 0 && input[i - 1] != ' ' && input_processed[input_processed_count - 1] != ' ') {
                                input_processed[input_processed_count++] = ' ';
                                string_count++;
                            }
                            if (i < inl && input[i] == '>' && input[i + 1] == '>') {
                                input_processed[input_processed_count++] = input[i++];
                                input_processed[input_processed_count++] = input[i];
                            } else {
                                input_processed[input_processed_count++] = input[i];
                            }
                            not_string[string_count] = 1;
                            command_not_completed = 1;
                            if (i < inl && input[i + 1] != ' ') {
                                input_processed[input_processed_count++] = ' ';
                                string_count++;
                            }
                        } else {
                            if (input[i] == ' ') {
                                string_count++;
                            }
                            if (command_not_completed && input[i] != '\n') {
                                command_not_completed = 0;
                            }
                            if (command_not_completed && input[i] == '\n') {
                                continue;
                            } else {
                                input_processed[input_processed_count++] = input[i];
                            }
                        }
                    }
                }
                if (single_quotes_opened || double_quotes_opened) {
                    quote_opened = 1;
                    msg(enter);
                } else if (command_not_completed && !syntax_error) {
                    msg(enter);
                } else {
                    quote_opened = 0;
                }
                free(input);
            }
            is_first_input = 0;
        }
        signal(SIGINT, &sigint_handler_parent_quit);
        if (input_not_empty && !syntax_error) {
            size_t inpl = strlen(input_processed);

            /* Parsing Background Character & */
            int is_background = 0;
            if (input_processed[inpl - 2] == '&') {
                is_background = 1;
                bg_command[bg_count] = (char *)malloc(sizeof(char) * MAX_LINE);
                if (bg_command[bg_count] == NULL) return 0;
                memset(bg_command[bg_count], 0, MAX_LINE);
                strcpy(bg_command[bg_count], input_processed);
                bg_command[bg_count][strlen(bg_command[bg_count]) - 1] = '\0';
            }
            if (input_processed[inpl - 1] == '\n') {
                input_processed[inpl - 1] = '\0';
            }

            /* Preparation for Pipes
             * @pipe_position: array for index of program
             */
            int pipe_position[MAX_PIPE];
            int pipe_count = 0; 
            pipe_position[pipe_count++] = 0;

            /* Preparation for pid */
            int pid_status[MAX_PIPE]; 
            for (int i = 0; i < MAX_PIPE; i++) {
                pid_status[i] = -1;
            }

            /* Preparation for I/O Error Handling */
            int io_error = 0;
            int duplicated_input = 0;
            int duplicated_output = 0;

            /* Parsing Arguments, Redirection, and Pipes + I/O Error Handling
             * @is_append: > : 0 / >> : 1
             * @argc: the number of arguments
             */
            int is_append = 0;
            int argc = 0;
            char *token = NULL;
            token = strtok(input_processed, " ");
            int string_track = 0;
            while (token != NULL) {
                if (not_string[string_track]) {
                    if (!strcmp(token, ">")) {
                        if (duplicated_output) {
                            errmsg("error", ": duplicated output redirection");
                            io_error = 1;
                            break;
                        } else {
                            duplicated_output = 1;
                        }
                        token = strtok(NULL, " ");
                        file_out = token;
                        string_track++;
                    } else if (!strcmp(token, ">>")) {
                        if (duplicated_output) {
                            errmsg("error", ": duplicated output redirection");
                            io_error = 1;
                            break;
                        } else {
                            duplicated_output = 1;
                        }
                        token = strtok(NULL, " ");
                        file_out = token;
                        is_append = 1;
                        string_track++;
                    } else if (!strcmp(token, "<")) {
                        if (duplicated_input) {
                            errmsg("error", ": duplicated input redirection");
                            io_error = 1;
                            break;
                        } else {
                            duplicated_input = 1;
                        }
                        token = strtok(NULL, " ");
                        file_in = token;
                        string_track++;
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
                        pipe_position[pipe_count++] = argc;
                    }
                } else if(strcmp(token, "&") != 0) {
                    argv[argc++] = token;
                }
                token = strtok(NULL, " ");
                string_track++;
            }
            argv[argc] = NULL;
            if (!io_error && argc == 0 && duplicated_output) {
                errmsg("error", ": missing program");
                io_error = 1;
            }

            if (!io_error) {
                /* Assigning Pipes to Programs */
                int pipes[MAX_PIPE];
                for (int i = 0; i < pipe_count - 1; i++) {
                    pipe(pipes + i * 2);
                }

                /* Running Child Processes */
                int tmpin = dup(0); // save stdin
                int tmpout = dup(1); // save stdout
                int fdin = 0;
                int fdout = 0;
                int child_process = 0;
                for (int i = 0; i < pipe_count; i++) {
                    /* Built-in Commands */
                    if (!strcmp(argv[pipe_position[i]], "exit")) {
                        msg("exit\n");
                        free(input_processed);
                        free(argv);
                        free(dir_current);
                        free(dir_before);
                        for(int j = 0; j < bg_count; j++) {
                            free(bg_command[j]);
                        }
                        exit(0);
                    }
                    if (!strcmp(argv[pipe_position[i]], "cd")) {
                        if (argv[pipe_position[i] + 1] == NULL || !strcmp(argv[pipe_position[i] + 1], "~")) {  // cd home directory
                            chdir(dir_home);
                            strcpy(dir_before, dir_current);
                            strcpy(dir_current, dir_home);
                        } else if (!strcmp(argv[pipe_position[i] + 1], "-")) {
                            chdir(dir_before);
                            char dir_temp[MAX_PATH];
                            strcpy(dir_temp, dir_current);
                            strcpy(dir_current, dir_before);
                            strcpy(dir_before, dir_temp);
                        } else {
                            if (chdir(argv[pipe_position[i] + 1]) < 0) {
                                errmsg(argv[pipe_position[i] + 1], ": No such file or directory");
                            } else {
                                strcpy(dir_before, dir_current);
                                char dir_temp[MAX_PATH];
                                getcwd(dir_temp, sizeof(dir_temp));
                                strcpy(dir_current, dir_temp);
                            }
                        }
                        continue;
                    }
                    if (!strcmp(argv[pipe_position[i]], "jobs")) {
                        for (int j = 0; j < bg_count; j++) {
                            if (waitpid(job_status[j * 2], NULL, WNOHANG) == 0) {
                                bgmsg(RUN, j + 1, bg_command[j]);
                            } else {
                                bgmsg(DONE, j + 1, bg_command[j]);
                            }
                        }
                        continue;
                    }

                    /* Begin Child Process */
                    pid_t pid = fork();
                    pid_status[i] = pid;

                    /* Background Process */
                    if (pid > 0 && is_background == 1 && i == 0) {
                        job_status[bg_count * 2] = pid;
                        job_status[bg_count * 2 + 1] = RUN;
                        bg_count++;
                        bgmsg(FALSE, bg_count, bg_command[bg_count - 1]);
                    }

                    /* Child Processes */
                    if (pid < 0) {
                        exit(0);
                    }
                    else if (pid == 0) {
                        signal(SIGINT, &sigint_handler_child);
                        /* Assign Pipes to Processes */
                        if (i + 1 < pipe_count) {
                            dup2(pipes[i * 2 + 1], 1);
                        }
                        if (i != 0) {
                            dup2(pipes[i * 2 - 2], 0);
                        }

                        /* Process Input Redirection */
                        if (i == 0 && file_in) {
                            fdin = open(file_in, O_RDONLY);
                            if (fdin <= 0 && errno == ENOENT) {
                                errmsg(file_in, ": No such file or directory");
                                exit(0);
                            }
                            dup2(fdin, 0);
                            close(fdin);
                        }

                        /* Process Output Redirection */
                        if (i + 1 == pipe_count && file_out) {
                            if (is_append) {
                                fdout = open(file_out, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
                                if (fdout <= 0 && (errno==EPERM || errno==EROFS)) {
                                    errmsg(file_out, ": Permission denied");
                                    exit(0);
                                }
                            } else {
                                fdout = open(file_out, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
                                if (fdout <= 0 && (errno==EPERM || errno==EROFS)) {
                                    errmsg(file_out, ": Permission denied");
                                    exit(0);
                                }
                            }
                            dup2(fdout, 1);
                            close(fdout);
                        }

                        /* Closing Pipes for Child Processes */
                        for (int k = 0; k < (pipe_count - 1) * 2; k++) {
                            close(pipes[k]);
                        }

                        /* pwd */
                        if (!strcmp(argv[pipe_position[i]], "pwd")) {
                            msg(dir_current);
                            msg("\n");
                            free(argv);
                            free(input_processed);
                            free(dir_current);
                            free(dir_before);
                            exit(0);
                        }

                        /* Bash Commands */
                        if(execvp(argv[pipe_position[i]], argv + pipe_position[i]) < 0) {
                            errmsg(argv[pipe_position[i]], ": command not found");
                        }
                        exit(0);
                    }
                }

                /* Closing Pipes */
                for (int i = 0; i < (pipe_count - 1) * 2; i++) {
                    close(pipes[i]);
                }
                
                /* Waiting for Child Process */
                if (is_background == 1) {
                    waitpid(job_status[(bg_count - 1) * 2], &child_process, WNOHANG);
                } else if(is_background == 0) {
                    for (int i = 0; i < pipe_count; i++) {
                        if (pid_status[i] != -1) {
                            waitpid(pid_status[i], NULL, WUNTRACED);
                        }
                    }
                }
                dup2(tmpin, 0);
                dup2(tmpout, 1);
                close(tmpin);
                close(tmpout);
            }
        }
        free(input_processed);
        free(argv);
        not_ctrl_c = 1;
    }
    for(int i = 0; i < bg_count; i++) {
        free(bg_command[i]);
    }
    if(dir_current != NULL) {
        free(dir_current);
    }
    if(dir_before != NULL) {
        free(dir_before);
    }
    return 0;
}
