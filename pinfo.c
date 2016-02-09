/********************************** For implementing pinfo command *****************************************/

#include "headers.h"

int pinfo(char** cmd_tokens) {
        int c, len=0;
        pid_t cpid;
        
        if(cmd_tokens[1] == NULL) cpid = getpid();
        else cpid = atoi(cmd_tokens[1]);
   
        char path[MAX_BUF_LEN]; // for generating path of proc/pid file
        char buf[MAX_BUF_LEN];  // for reading file line by line
        
        printf("pid -- %d\n", cpid);
        
        sprintf(path, "/proc/%d/status", cpid);
        FILE* fp = fopen(path, "r");
        if(!fp) {
                perror("Error while opening the proc/pid/status file\n"); 
        }
        else {
                char state;
                fgets(buf,256, fp);
                fgets(buf, 256, fp);
                sscanf(buf, "State:\t%c", &state);
                printf("Process Status -- %c\n", state);
                fclose(fp);
        }
        
        sprintf(path, "/proc/%d/statm", cpid);
        fp = fopen(path, "r");
        if(!fp) {
                perror("Error while opening the proc/pid/statm file\n");

        }
        else {
                while((c=fgetc(fp))!=' ') buf[len++] = c;
                buf[len] = '\0';
                printf("Memory -- %s\n", buf);
                fclose(fp);
        }
        
        sprintf(path, "/proc/%d/exe", cpid);
        len = -1;
        len = readlink(path, buf, MAX_BUF_LEN - 1);
        if(len==-1) {
                perror("Error while opening the proc/pid/exe file\n");
        }
        else {
                buf[len] = '\0';
                mod_cwd_rel(buf);
                printf("Executable Path -- %s\n", buf);
        }
}
