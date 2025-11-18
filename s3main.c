#include "s3.h"

int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];

    ///The last (previous) working directory 
    char lwd[MAX_PROMPT_LEN-6]; 

    init_lwd(lwd);///Implement this function: initializes lwd with the cwd (using getcwd)

    //Stores pointers to command arguments.
    ///The first element of the array is the command name.
    char *args[MAX_ARGS];

    ///Stores the number of arguments
    int argsc;

    while (1){
        //continues added to be safe, and falling into default logic

        read_command_line(line, lwd); ///Notice the additional parameter (required for prompt construction)
        //Note note sure why we need lwd
        //read_command_line(line);

        //main rceent change has been launch -> return pid , so can call reap(pid)

        if (line[0] == '\0'){
        // probably due to unterminated quote or empty input
        continue;
        }

        if (command_has_pipes(line)){
            char line_copy[MAX_LINE];
            strncpy(line_copy, line, sizeof(line_copy));
            line_copy[sizeof(line_copy)-1] = '\0';

            char *stages[MAX_ARGS];
            int n = 0;

            split_pipeline(line_copy, stages, &n);

            if (n > 0){
                launch_pipeline(stages, n);
            }
            continue;
        }

        else if (is_cd(line)){
            parse_command(line, args, &argsc);
            run_cd(args, argsc, lwd);
            continue;
        }


        else{
            parse_command(line, args, &argsc);
            if (argsc == 0){
                continue;
            }
            pid_t pid = launch_program_with_redirection(args, argsc);
            reap(pid);
        }
    }

    return 0;
}
