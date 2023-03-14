//
// Created by franzozich on 06/01/23.
//

#ifndef UNTITLED4_MY_UTILS_H
#define UNTITLED4_MY_UTILS_H

#endif //UNTITLED4_MY_UTILS_H

#define MAX_CMD_SIZE 256
#define MAX_MSG_SIZE 100

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/wait.h>
#include<time.h>
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<ctype.h>
#include<unistd.h>

void message();
int pipes_size(char** pipes);
int output_redirection(char *pipe);
int input_redirection(char *pipe);
char** break_to_pipe(char * str);
char** break_to_line(char *str);
char* read_usr_input(void);
void run_usr_command (char *pipe, int fd0, int fd1);
void cmd_handler(int size,char **mypipes);
char* remove_redirection_operator(char *pipe);
