/**************************************************Main shell loop **************************************************/

#include "headers.h"

int main()
{
        //Basic Setup
        initializer();

        //Command Loop
        while(1) {
                if(signal(SIGCHLD,signal_handler)==SIG_ERR)
                        perror("can't catch SIGCHLD");
                if(signal(SIGINT,signal_handler)==SIG_ERR)
                        perror("can't catch SIGINT!");

                set_prompt();

                int i,j;

                char** cmds = malloc((sizeof(char)*MAX_BUF_LEN)*MAX_BUF_LEN); // array of semi-colon separated commands

                for(j = 0; j < MAX_BUF_LEN; j++) cmds[j] = '\0';

                char* cmdline = read_cmdline(); // read command line
                int num_cmds = parse_cmd_line(cmdline, cmds); // parse command line

                for(i = 0; i < num_cmds; i++) {
                        infile = outfile = NULL;
                        is_bg = 0, num_pipe = 0;
                        char* cmd_copy = strdup(cmds[i]);

                        char** cmd_tokens = malloc((sizeof(char)*MAX_BUF_LEN)*MAX_BUF_LEN); // array of command tokens
                        for(j = 0; j < MAX_BUF_LEN; j++) cmd_tokens[j] = '\0';

                        if(check_for_pipe(strdup(cmds[i])) == -1) {
                                if(input_redi == 1 || output_redi == 1) normal_cmd(parse_for_redi(strdup(cmd_copy), cmd_tokens), cmd_tokens, cmd_copy);
                                else {
                                        int tokens = parse_cmd(strdup(cmds[i]), cmd_tokens);
                                //        if(strcmp(cmd_tokens[0], "quit\0") == 0) exit(0);
                                        normal_cmd(tokens, cmd_tokens, cmd_copy);
                                }
                        }
                        else redi_and_pipi_cmd(cmds[i]);
                }
                if(cmds) free(cmds);
                if(cmdline) free(cmdline);
        }
        return 0;
}
