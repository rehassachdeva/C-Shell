/********************************** For input output redirection ***************************/

#include "headers.h"

int open_infile() {
        int f = open(infile, O_RDONLY, S_IRWXU);
        if (f < 0) {
                perror(infile);
                
        }
        dup2(f, STDIN_FILENO);
        close(f);
        return f;
}

int open_outfile() {
        int f;
        if(last == 1) f = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
        else if(last == 2) f = open(outfile, O_CREAT | O_WRONLY | O_APPEND, S_IRWXU);
        if(f < 0) {
                perror(outfile);
        }
        dup2(f, STDOUT_FILENO);
        close(f);
        return f;
}
