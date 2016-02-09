/********************************** For parsing command line and commands ***************************/

#include "headers.h"

char* read_cmdline() {
        int len=0,c;            
        char* cmd = malloc(sizeof(char)*MAX_BUF_LEN);
        while(1) {
                c = getchar();
                if(c == '\n') {
                        cmd[len++] = '\0';
                        break;
                }
                else
                        cmd[len++] = c;
        }
        return cmd;
}

int parse_cmd_line(char* cmdline, char** cmds) {
        int num_cmds = 0;
        char* token = strtok(cmdline, ";");
        while(token!=NULL) {
                cmds[num_cmds++] = token;
                token = strtok(NULL, ";");
        }
        return num_cmds;
}

int parse_cmd(char* cmd, char** cmd_tokens) {
        int tok = 0;
        char* token = strtok(cmd, CMD_DELIMS);
        while(token!=NULL) {
                cmd_tokens[tok++] = token;
                token = strtok(NULL, CMD_DELIMS);
        }
        return tok;
}

int check_for_pipe(char* cmd) {
        int i;
        idxi = idxo = last = piping = input_redi = output_redi = 0;
        for( i = 0 ; cmd[i] ; i++) {
                if(cmd[i] == '|') {
                        piping = 1;
                }
                if(cmd[i] == '<') {
                        input_redi = 1;
                        if(idxi == 0 ) idxi = i;
                }
                if(cmd[i] == '>') {
                        output_redi = 1;
                        if(last == 0) last = 1;
                        if(idxo == 0 ) idxo = i;
                }
                if(cmd[i] == '>' && cmd[i+1] == '>') last = 2;
        }
        if(piping) return 1;
        else return -1;
}

void parse_for_piping(char* cmd) {
        char* copy = strdup(cmd);
        char* token;
        int tok = 0;
        token = strtok(copy, "|");
        while(token!= NULL) {
                pipe_cmds[tok++] = token;
                token = strtok(NULL, "|");
        }
        num_pipe = tok;
}

int parse_for_redi(char* cmd, char** cmd_tokens) {
        char* copy = strdup(cmd);
        idxi = idxo = last = input_redi = output_redi = 0;
        infile = outfile = NULL;
        int i, tok = 0;
        for( i = 0 ; cmd[i] ; i++) {
                if(cmd[i] == '<') {
                        input_redi = 1;
                        if(idxi == 0 ) idxi = i;
                }
                if(cmd[i] == '>') {
                        output_redi = 1;
                        if(last == 0) last = 1;
                        if(idxo == 0 ) idxo = i;
                }
                if(cmd[i] == '>' && cmd[i+1] == '>') last = 2;
        }
        if(input_redi == 1 && output_redi == 1) {
                char* token;
                token = strtok(copy, " <>\t\n");
                while(token!=NULL) {
                        cmd_tokens[tok++] = strdup(token);
                        token = strtok(NULL, "<> \t\n");
                }
                if(idxi < idxo ) {
                        infile = strdup(cmd_tokens[tok - 2]);
                        outfile = strdup(cmd_tokens[tok - 1]);
                }
                else {
                        infile = strdup(cmd_tokens[tok - 1]);
                        outfile = strdup(cmd_tokens[tok - 2]);
                }
                cmd_tokens[tok - 2] = cmd_tokens[tok - 1] = NULL;
                
                return tok - 2;
        }
                
        if(input_redi == 1) {
          char* token;
          char* copy = strdup(cmd);
 
          char** input_redi_cmd = malloc((sizeof(char)*MAX_BUF_LEN)*MAX_BUF_LEN);
          token = strtok(copy, "<");
          while(token!=NULL) {
          input_redi_cmd[tok++] = token;
          token = strtok(NULL, "<");
          }
          copy = strdup(input_redi_cmd[tok - 1]);

          token = strtok(copy, "> |\t\n");
          infile = strdup(token);

          tok = 0;
          token = strtok(input_redi_cmd[0], CMD_DELIMS);
          while(token!=NULL) {
          cmd_tokens[tok++] = strdup(token);
          token = strtok(NULL, CMD_DELIMS);
          }

          cmd_tokens[tok] = NULL;

          free(input_redi_cmd);    
        }

        if(output_redi == 1) {
                char* copy = strdup(cmd);
                char* token;
                char** output_redi_cmd = malloc((sizeof(char)*MAX_BUF_LEN)*MAX_BUF_LEN);
                if(last == 1)
                        token = strtok(copy, ">");
                else if(last == 2)
                        token = strtok(copy, ">>");
                while(token!=NULL) {
                        output_redi_cmd[tok++] = token;
                        if(last == 1) token = strtok(NULL, ">");
                        else if(last == 2) token = strtok(NULL, ">>");
                }

                copy = strdup(output_redi_cmd[tok - 1]);
                token = strtok(copy, "< |\t\n");
                outfile = strdup(token);

                tok = 0;
                token = strtok(output_redi_cmd[0], CMD_DELIMS);
                while(token!=NULL) {
                        cmd_tokens[tok++] = token;
                        token = strtok(NULL, CMD_DELIMS);
                }

                free(output_redi_cmd);    
        }
        if(input_redi == 0 && output_redi == 0 ) return parse_cmd(strdup(cmd), cmd_tokens);
        else return tok;
}
