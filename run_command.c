/********************************************** For running a command using fork and exec *********************************/

#include "headers.h"

int run_cmd(char** cmd_tokens) {
        pid_t pid;
        pid = fork();
        if(pid < 0) {
                perror("Child Proc. not created\n");
                return -1; 
        }
        else if(pid==0) { 
                int fin, fout;
                setpgid(pid, pid);                               /* Assign pgid of process equal to its pid */
                
                if(input_redi) {
                        fin = open_infile();
                        if(fin == -1) _exit(-1);
                }
                if(output_redi) {
                        fout = open_outfile();
                      if(fout == -1) _exit(-1);
                }

                if(is_bg == 0) tcsetpgrp(shell, getpid());        /* Assign terminal to this process if it is not background */

                signal (SIGINT, SIG_DFL);                         /* Restore default signals in child process */
                signal (SIGQUIT, SIG_DFL);
                signal (SIGTSTP, SIG_DFL);
                signal (SIGTTIN, SIG_DFL);
                signal (SIGTTOU, SIG_DFL);
                signal (SIGCHLD, SIG_DFL);
                
                int ret;
                if((ret = execvp(cmd_tokens[0], cmd_tokens)) < 0) {
                        perror("Error executing command!\n");
                        _exit(-1);
                }
                _exit(0);
        }
        if(is_bg == 0) {
                tcsetpgrp(shell, pid);                              /* Make sure the parent also gives control to child */
                add_proc(pid, cmd_tokens[0]);
                int status;
                fgpid = pid;
                waitpid(pid, &status, WUNTRACED);               /* Wait for this process, return even if it has stopped without trace */
                
                if(!WIFSTOPPED(status)) rem_proc(pid);         /* returns true if the child process was stopped by delivery of a signal */
         
                else fprintf(stderr, "\n%s with pid %d has stopped!\n", cmd_tokens[0], pid);
                
                tcsetpgrp(shell, my_pgid);                     /* Give control of terminal back to the executable */
        }
        else {

                printf("\[%d] %d\n", num_jobs, pid);
                add_proc(pid, cmd_tokens[0]);
                return 0;
        }
}


void add_proc(int pid, char* name) {
        table[num_jobs].pid = pid;
        table[num_jobs].name = strdup(name);
        table[num_jobs].active = 1;
        num_jobs++;
}

void rem_proc(int pid) {
        int i;
        for(i = 0 ; i < num_jobs; i++) {
                if(table[i].pid == pid) {
                        table[i].active = 0;
                        break;
                }
        }
}
void normal_cmd(int tokens, char** cmd_tokens, char* cmd_copy) {
        if(tokens > 0) {
                if(strcmp(cmd_tokens[0], "fg\0") == 0 ) fg(tokens, cmd_tokens);
                else if(strcmp(cmd_tokens[0], "overkill\0") == 0) overkill();
                else if(strcmp(cmd_tokens[0], "jobs\0") == 0) jobs();
                else if(strcmp(cmd_tokens[0], "kjob\0") == 0) kjob(tokens, cmd_tokens);
                else if(strcmp(cmd_tokens[tokens-1], "&\0") == 0) {
                        cmd_tokens[tokens - 1] = NULL;
                        is_bg = 1;
                        run_cmd(cmd_tokens);        // for running background process
                }
                else if(strcmp(cmd_tokens[0], "cd\0") == 0) cd_cmd(cmd_tokens, cwd, base_dir);
                else if(strcmp(cmd_tokens[0], "pwd\0") == 0) pwd(cmd_tokens);
                else if(strcmp(cmd_tokens[0], "quit\0") == 0) {
                        _exit(0);
                }
                else if(strcmp(cmd_tokens[0], "echo\0") == 0) echo(cmd_tokens, tokens, cmd_copy);
                else if(strcmp(cmd_tokens[0], "pinfo\0") == 0) pinfo(cmd_tokens);
                else if(isalpha(cmd_tokens[0][0]))
                        run_cmd(cmd_tokens);
        }
        free(cmd_tokens);
}

void redi_and_pipi_cmd(char* cmd) {
        int pid, pgid, fin, fout;

        num_pipe = 0;

        parse_for_piping(cmd);

        int* pipes = (int* )malloc(sizeof(int)*(2*(num_pipe - 1)));

        int i;

        for(i = 0; i < 2*num_pipe - 3; i += 2) {
                if(pipe(pipes + i) < 0 ) {             /* Create required number of pipes, each a combination of input and output fds */
                        perror("Pipe not opened!\n");
                        return;
                }
        }
        int status,j;
        for(i = 0; i < num_pipe ; i++) {
                char** cmd_tokens = malloc((sizeof(char)*MAX_BUF_LEN)*MAX_BUF_LEN); /* array of command tokens */
                int tokens = parse_for_redi(strdup(pipe_cmds[i]), cmd_tokens);
                is_bg = 0;               
                pid = fork();
                if(i < num_pipe - 1)
                        add_proc(pid, cmd_tokens[0]);
                
                if(pid != 0 ) {
                        if(i == 0 ) pgid = pid;
                        setpgid(pid, pgid);                         /* Assign pgid of process equal to pgid of first pipe command pid */
                }
                if(pid < 0) {
                        perror("Fork Error!\n");
                }
                else if(pid == 0) {
                        signal (SIGINT, SIG_DFL);                              /* Restore default signals in child process */
                        signal (SIGQUIT, SIG_DFL);
                        signal (SIGTSTP, SIG_DFL);
                        signal (SIGTTIN, SIG_DFL);
                        signal (SIGTTOU, SIG_DFL);
                        signal (SIGCHLD, SIG_DFL);

                        if(output_redi) fout = open_outfile();
                        else if(i < num_pipe - 1) dup2(pipes[2*i + 1], 1);

                        if(input_redi) fin = open_infile();
                        else if(i > 0 ) dup2(pipes[2*i -2], 0);
                 
                        int j;
                        for(j = 0; j < 2*num_pipe - 2; j++) close(pipes[j]);
         
                        if(execvp(cmd_tokens[0], cmd_tokens) < 0 ) {
                                perror("Execvp error!\n");
                                _exit(-1);
                        }
                }
        }
 
        for(i = 0; i < 2*num_pipe - 2; i++) close(pipes[i]);
 
        if(is_bg == 0) {
                tcsetpgrp(shell, pgid);                  /* Assign terminal to pg of the pipe commands */

                for(i = 0; i < num_pipe ; i++) {

                        int cpid = waitpid(-pgid, &status, WUNTRACED);
                        
                        /* Wait for this process, return even if it has stopped without trace */

                        if(!WIFSTOPPED(status)) rem_proc(cpid);
                }

                tcsetpgrp(shell, my_pgid);            /* Give control of terminal back to the executable */
        }
}
