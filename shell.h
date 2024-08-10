#ifndef _SHELL_H_
#define _SHELL_H_

char **read_line(void);
int execute(char **args);
int execute2(int in, int out, char** args);
int pipes (int n, char *** array);
void handler(int sig);
int nyush();

//Built-in
int builtin_cd(char **args);
int builtin_exit();

#endif
