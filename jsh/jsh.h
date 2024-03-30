#ifndef JSH_H
#define JSH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <pwd.h>

#define MAX_LINE 1024
#define MAX_PIPE 100
#define MAX_PATH 260
#define MAX_JOBS 100

#define FALSE 0
#define DONE 1
#define RUN 2

#endif
