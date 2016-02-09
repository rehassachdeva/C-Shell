/*************************** Functions for implementing built-in commands *************************************/

#include "headers.h"

int cd_cmd(char** cmd_tokens, char* cwd, char* base_dir) {
        if(cmd_tokens[1] == NULL || strcmp(cmd_tokens[1], "~\0") == 0 || strcmp(cmd_tokens[1], "~/\0") == 0) {
                chdir(base_dir);
                strcpy(cwd, base_dir);
                mod_cwd_rel(cwd);
        }
        else if(chdir(cmd_tokens[1]) == 0) {
                getcwd(cwd, MAX_BUF_LEN);
                mod_cwd_rel(cwd);
                return 0;
        }
        else {
                perror("Error executing cd command");
        }
}

void echo(char** cmd_tokens, int tokens, char* cmd) {
        if(tokens > 1 && cmd_tokens[1][0] == '-') {
                run_cmd(cmd_tokens);
                return;
        }
        int i, len = 0, in_quote = 0, flag = 0;
        char buf[MAX_BUF_LEN] = "\0";
        for(i = 0; isspace(cmd[i]); i++);
        if(i == 0) i = 5;
        else i += 4;
        for(; cmd[i] != '\0' ; i++) {
                if(cmd[i] == '"') {
                        in_quote = 1 - in_quote;
                }    
                else if(in_quote == 0 && (isspace(cmd[i])) && flag == 0) {
                        flag = 1;
                        if(len > 0) buf[len++] = ' ';
                }
                else if(in_quote == 1 || !isspace(cmd[i])) buf[len++] = cmd[i];
                if(!isspace(cmd[i]) && flag == 1) flag = 0;
        }
        if(in_quote == 1) {
                fprintf(stderr, "Missing quotes\n");
                return;
        }
        else printf("%s\n", buf);
}

void pwd(char** cmd_tokens) {
        char pwd_dir[MAX_BUF_LEN];
        getcwd(pwd_dir, MAX_BUF_LEN - 1); 
        if(cmd_tokens[1] == NULL) printf("%s\n", pwd_dir);
        else run_cmd(cmd_tokens);
}

void jobs() {
        int i;
        for(i = 0; i < num_jobs ; i++) {
                if(table[i].active == 1) {
                        printf("[%d] %s [%d]\n", i, table[i].name, table[i].pid);
                }
        }
}

void kjob(int tokens, char** cmd_tokens) {
        if(tokens != 3) {
                fprintf(stderr, "Invalid usage of kjob!\n");
                return;
        }
        int job_num = atoi(cmd_tokens[1]);
        if(table[job_num].active == 1) {
                if(kill(table[job_num].pid, atoi(cmd_tokens[2])) < 0)                 /* For sending signal mentioned to job mentioned */
                        fprintf(stderr, "Signal not sent!\n");
        }
        else fprintf(stderr, "Job not found\n");               
}

void overkill() {
        int i;
        for(i = 0 ; i < num_jobs ; i++) {
                if(table[i].active == 1) {
                        if(kill(table[i].pid, SIGKILL) < 0)                           /* Kill all jobs */
                                perror("Error killing process!\n");
                }
        }
}

void fg(int tokens, char** cmd_tokens) {
        if(tokens != 2) {
                fprintf(stderr, "Invalid usage of fg");
                return;
        }       

        int i, job_num = atoi(cmd_tokens[1]), status;
        if(table[job_num].active == 0) {
                printf("No such job exists\n");
                return;
        }
        if(table[job_num].active == 1) {
                int pid = table[job_num].pid, pgid;
                pgid = getpgid(pid);                     /* get pgid of mentioned job */
                tcsetpgrp(shell, pgid);                  /* Give control of shell's terminal to this process */

                fgpid = pgid;                            /* Set this pgid as fg pgid */
                if(killpg(pgid, SIGCONT) < 0)            /* Send signal to thid pgid to continue if stopped */
                        perror("Can't get in foreground!\n");
                waitpid(pid, &status, WUNTRACED);        /* Wait for this process, return even if it has stopped without trace */
                if(!WIFSTOPPED(status)) {                /* returns true if the child process was stopped by delivery of a signal */
                        table[job_num].active = 0;
                        fgpid = 0;
                }
                tcsetpgrp(shell, my_pid);                /* Give control of terminal back to the executable */
        }
        else fprintf(stderr, "No job found\n");
}
