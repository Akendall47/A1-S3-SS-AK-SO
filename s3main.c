#include "s3.h"

// int main(int argc, char *argv[]){

//     ///Stores the command line input
//     char line[MAX_LINE];

//     ///The last (previous) working directory 
//     char lwd[MAX_PROMPT_LEN-6]; 

//     init_lwd(lwd);///Implement this function: initializes lwd with the cwd (using getcwd)

//     //Stores pointers to command arguments.
//     ///The first element of the array is the command name.
//     char *args[MAX_ARGS];

//     ///Stores the number of arguments
//     int argsc;

//     while (1) {
//         trim_whitespace(line);
//         read_command_line(line, lwd); ///Notice the additional parameter (required for prompt construction)
//         //Note not sure why we need lwd
//         //read_command_line(line);

//         //Have to use strtok_r because the function calls of strtok inside the functions mess up the pointers
//         char *saveptr;  
//         char *token = strtok_r(line, ";", &saveptr);

//         while (token != NULL) {
//             argsc = 0;
//             trim_whitespace(token);
//             printf("%s", token);
//             // Check if token is subshell
//             // Parse token to new shell
//             if (is_subshell(token)){
//                 parse_command(token, args, &argsc);
                
//             }
//             else if (is_cd(token)) {
//                 parse_command(token, args, &argsc);
//                 run_cd(args, argsc, lwd);
//             } else if (command_with_redirection(token)) {
//                 parse_command(token, args, &argsc);
//                 launch_program_with_redirection(args, argsc);
//                 reap();
//             } else {
//                 parse_command(token, args, &argsc);
//                 launch_program(args, argsc);
//                 reap();
//             }

//             token = strtok_r(NULL, ";", &saveptr);
//         }


//     }

//     return 0;
// }

int main(int argc, char *argv[]) {
    char line[MAX_LINE];
    char lwd[MAX_PROMPT_LEN-6]; 
    init_lwd(lwd);

    while (1) {
        read_command_line(line, lwd);
        process_input(line, lwd); // Calls the function we created above
    }
    return 0;
}