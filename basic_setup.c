/****************************** For setting up user, base directory, prompt, signal handler, pgid etc **********************/

#include "headers.h"

void get_prompt_vars() {
        getlogin_r(user, MAX_BUF_LEN - 1); 
        gethostname(hostname, MAX_BUF_LEN - 1); 
}

void get_home_dir() {
        getcwd(base_dir,  MAX_BUF_LEN - 1); 
        strcpy(cwd, base_dir);
}

void set_prompt() {
        printf("%s@%s:%s$ ", user, hostname, cwd);
}

void mod_cwd_rel(char* cwd) {
        int i, j;
        for(i = 0; cwd[i]==base_dir[i] && cwd[i]!='\0' && base_dir[i] != '\0'; i++);
        if(base_dir[i] == '\0') {
                cwd[0] = '~';
                for(j = 1; cwd[i]!='\0'; j++) {
                        cwd[j] = cwd[i++];
                }
                cwd[j] = '\0';
        }
}

void signal_handler(int signum) {
        if(signum == SIGINT) {
                signal(SIGINT,SIG_IGN);               /* For ignoring ctrl + c */
                signal(SIGINT, signal_handler);       /* For re-setting signal handler */
        }
        else if(signum == SIGCHLD) {                  /* For handling signal from child processes */
                int i, status, die_pid;
                while((die_pid = waitpid(-1, &status, WNOHANG)) > 0) {  /* Get id of the process which has terminated  */
                        for(i = 0; i < num_jobs; i++)
                        {
                                if(table[i].active==0) continue;
                                else if(table[i].pid == die_pid)
                                        break;
                        }
                        if(i != num_jobs) {
                        if(WIFEXITED(status))          /* returns true if the child terminated normally */
                                fprintf(stdout, "\n%s with pid %d exited normally\n", table[i].name, table[i].pid);
                        else if(WIFSIGNALED(status))   /* returns true if the child process was terminated by a signal */
                                fprintf(stdout, "\n%s with pid %d has exited with signal\n", table[i].name, table[i].pid);
                        table[i].active = 0;
                        }
                }
        }
}

void initializer() {

        shell = STDERR_FILENO;                         /* FD for stderr */

        num_jobs = 0;

        input_cmd_tokens = malloc((sizeof(char)*MAX_BUF_LEN)*MAX_BUF_LEN);
        output_cmd_tokens = malloc((sizeof(char)*MAX_BUF_LEN)*MAX_BUF_LEN);            /* Initialisations and allocations */

        if(isatty(shell)) {                                                         /* test whether a stderr refers to a terminal */
                while(tcgetpgrp(shell) != (shell_pgid = getpgrp()))                  /* if it does, send signal to make process \
                                                                                 group or executable same as process group of stderr */
                        kill(shell_pgid, SIGTTIN);                             /* SIGTTIN sets terminal input for background processes */
        }

        signal (SIGINT, SIG_IGN);                                    /* To ignore Ctrl c */

        signal (SIGTSTP, SIG_IGN);                                   /* To ignore Ctrl z */

        signal (SIGQUIT, SIG_IGN);                                   /* To ignore Ctrl \ */

        signal (SIGTTIN, SIG_IGN);                                   /* To ignore background processes */
        
        signal (SIGTTOU, SIG_IGN);

        my_pid = my_pgid = getpid();                                 /* Set pgid of executable same as pid */
        setpgid(my_pid, my_pgid);
        tcsetpgrp(shell, my_pgid);                                   /* Give control of stderr to executable's process group */
        
        get_prompt_vars();
        get_home_dir();
        mod_cwd_rel(cwd);                                 /* modify current working directory relative to assumed home directory */
}
