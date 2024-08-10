#include <stdio.h>
#include "string.h"
#include "stdlib.h"
#include "sys/wait.h"
#include "unistd.h"
#include <dirent.h>
#include <fcntl.h>
#include<signal.h>

#include "shell.h"

char *command_names[] = {"cd", "exit"};
int (*commands[])(char**) = {&builtin_cd, &builtin_exit};

int nyush()
{
    signal(SIGINT, handler);
    signal(SIGQUIT, handler);
    signal(SIGTSTP, handler);
    char **input;
    int flag = 1;
    char dir[1024];
    char *token;
    char *prev;
    while(flag){
        int stdinput = dup(STDIN_FILENO);
        int stdoutput = dup(STDOUT_FILENO);
        getcwd(dir, 1024);
        token = strtok(dir, "/");
        while(token != NULL){
            prev = token;
            token = strtok(NULL, "/");
        }
        if(strcmp(prev, "home")==0){
            prev = "/";
        }
        
        printf("[nyush %s]$ ", prev);
        fflush(stdout);
        input = read_line();
        flag = execute(input);
        free(input);
        if(stdinput){
            dup2(stdinput, STDIN_FILENO);
            close(stdinput);
        }
        if(stdoutput){
            dup2(stdoutput, STDOUT_FILENO);
            close(stdoutput);
        }
        }

    return 0;
}

char **read_line(void){
    char *line;
    size_t bufsize = 0;
    
    if(getline(&line, &bufsize, stdin) == -1){
        if(feof(stdin)){
            exit(EXIT_SUCCESS);
        }
        else{
            exit(EXIT_FAILURE);
        }
    }
    
    char **args = malloc(bufsize*sizeof(char));
    char *token;
    size_t i = 0;
    
    token = strtok(line, "\n");
    token = strtok(line, " ");
    while(token != NULL){
        args[i] = strdup(token);
        i = i + 1;
        
        if( i >= bufsize){
            bufsize = bufsize + 64;
            args = realloc(args, bufsize*sizeof(char));
            
        }
        token = strtok(NULL, " ");
    }
    return args;
}


int execute(char **args){
    //Blank command
    if(strcmp("\n",args[0])==0){
        return 1;
    }
    //Built ins
    size_t i;
    for(i=0; i < sizeof(command_names)/sizeof(char *); i++){
        
        if (strcmp(args[0], command_names[i]) == 0) {
            return (*commands[i])(args);
        }

    }
    //Check for pipes
    int pipe = 0;
    int p = 0;
    while(args[p] != NULL){
        if(strcmp(args[p],"|")==0){
            pipe = 1;
            break;
        }
        p = p +1;
    }
    if(pipe){
        int i2 = 0;
        int j = 0;
        int k = 0;
        int indexLen = 0;
        int *indexes = malloc(indexLen*sizeof(int));
        while(args[i2] != NULL){

            if(strcmp(args[i2], "|")==0){
                if( k >= indexLen){
                    indexLen = indexLen + 1;
                    indexes = realloc(indexes, indexLen*sizeof(int));
                }
                indexes[k] = j;
                j = i2+1;
                k = k +1;
                
                
            }
            i2 = i2 +1;
        }
        indexes[k]=j;
        indexes[k+1] = i2+1;
        indexLen = indexLen+1;

        char *** array = (char***) calloc((indexLen+1),sizeof(char*));
        for(int m = 0; m < indexLen; m++){
            int l = indexes[m+1]-indexes[m];
            array[m] = (char**) calloc((l+1),sizeof(char*));

            for(int n = 0; n < indexes[m+1]-indexes[m]-1; n++){
                array[m][n] = args[n+indexes[m]];
            }
        }
        return pipes(indexLen, array);
    }
    
    return execute2(0, 1, args);
}

int builtin_cd(char **args)
{
    if (args[1] == NULL || args[2] != NULL) {
        printf("Error: invalid command\n");
        } 
    else {
        if (chdir(args[1]) != 0) {
            printf("Error: invalid directory\n");
            }
    }
  return 1;
}

int builtin_exit(char **args)
{
    if(args[1] != NULL){
        return 1;
    }
    return 0;
}

int execute2 (int in, int out, char** args)
{
    pid_t pid =fork();
    int status;
    int i = 0;
    char input[64];
    char output[64];
    if (pid  == 0)
    {
        if (in != 0)
        {
            dup2 (in, STDIN_FILENO);
            close (in);
        }
        if (out != 1)
        {
            dup2 (out, STDOUT_FILENO);
            close (out);
        }
        while(args[i] != NULL) 
        {
            if(strcmp(args[i],"<")==0)
            {        
                if(i-1 < 0){
                    printf("Error: invalid command\n");
                }
                else if(args[i+1]==NULL){
                    printf("Error: invalid command\n");
                }
                else{
                    args[i]=NULL;
                    strcpy(input,args[i+1]);
                    i=i+1;

                    int fdin = open(input, O_RDONLY, 0);
                    dup2(fdin, STDIN_FILENO);
                    close(fdin);
                }
            }
            else if(strcmp(args[i],">")==0)
            {      
                if(i-1 < 0){
                    printf("Error: invalid command\n");
                    return 1;
                }
                else if(args[i+1]==NULL){
                    printf("Error: invalid command\n");
                    return 1;
                }
                else{
                    args[i]=NULL;
                    strcpy(output,args[i+1]);
                    i=i+1;

                    int fdout = open(output,O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR);
                    dup2(fdout, 1);
                    close(fdout);
                    
                }
            }        
            else if(strcmp(args[i],">>")==0)
            {      
                if(i-1 < 0){
                    printf("Error: invalid command\n");
                    return 1;
                }
                else if(args[i+1]==NULL){
                    printf("Error: invalid command\n");
                    return 1;
                }
                else{
                    args[i]=NULL;
                    strcpy(output,args[i+1]);
                    i=i+1;

                    int fdout = open(output, O_CREAT|O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
                    dup2(fdout, 1);
                    close(fdout);
                    
                }
            }
            i=i+1;
        }
        fflush(stdout);
        signal(SIGINT, handler);
        signal(SIGQUIT, handler);
        signal(SIGTSTP, handler);
        if (execvp(args[0], args) == -1) {
            printf("Error: invalid command\n");
        }
        exit(EXIT_FAILURE);
    }
    else if(pid < 0){
        printf("Error: forking error\n");
    }
    else{
        waitpid(pid, &status, WUNTRACED);
    }
    return 1;
}

int pipes (int n, char *** array)
{
    int i;
    int in, fd [2];
    in = 0;
    for (i = 0; i < n - 1; ++i)
    {
        pipe (fd);
        execute2 (in, fd [1], array[i]);
        close (fd [1]);
        in = fd [0];
    }
    if (in != 0)
    {
        dup2 (in, 0);
        close(in);
    }
    close(fd[0]);
    close(fd[1]);
    
    return execute2(0, 1, array[i]);
}

void handler(int sig){
    if(sig==SIGINT){
        printf("\n");
    }
    else if(sig ==SIGQUIT){
        printf("Quit (core dumped)\n");
    }
    else if(sig== SIGTSTP){
        printf("\n");
    }
    
}