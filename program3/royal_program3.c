#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

#define max_line_length 2048
#define max_num_args 512
//set parameters for maximum number of arguments and line length.

//set up stuff needed up here

//global scope vars
char* args[max_num_args];
int total_processes = 0;
int is_background = 0;
int foreground_only = 1;
int status;
//int processes[max_processes]; add once prof tells you this number

//function prototypes
//Will need custom handlers for 2 signals, SIGINT and SIGTSTP
int parse_commands();   //should return how many args it found
void check_args(int); //should basically check arguments and if exit, \n, or # do the appropriate else, check the advances commands.
void exit_shell();   //can't just exit command, need to kill off all processes.
void run_cd();
void run_status(int*);
void execute_command(int, int*);
char* searchInPath(char*);


int main(){
    //first things first set up loop to continue till exit status.
    //Then we'll parse commands into array.
    //test commands for comments or empty lines.
    //go from there.
    printf("$ smallsh\n");
    fflush(stdout);
    char* args_line[max_line_length]; //to get args from command line.
    int num_args;

    while(1){
        //create way to parse command line, return number of args and get args in array
        num_args = parse_commands(args_line);
        args[max_num_args] = NULL; //end the array of arguments
        check_args(num_args);
        //do something to test check the command and do that appropiate thing with them
    }

    return 0;
}


int parse_commands(char* args_line){
    // char temp[max_line_length]; // to expand $$.
    int num_args = 0;
    char temp[max_line_length];
    printf(": ");   //set up printing the command line
    fflush(stdout); //supposed to call this every time we print
    //now we want to read in from the command line and probably use str to k to break apart the commands and store in array_args.
    //fgets syntax from https://www.tutorialspoint.com/c_standard_library/c_function_fgets.htm
    fgets(args_line, max_line_length, stdin);
    strtok(args_line, "\n"); //start parsing till new line.

    char* token = strtok(args_line, " "); //split by space to find each arg 
    while(token != NULL){//now I need to parse and add commands into array
        args[num_args] = token;
        //check next spot and increment num args to add to args array
        //Handles expansion of $$
        for(int i = 0; i < strlen(args[num_args]); i++){
            char* temp = strdup(args[num_args]);
            if(args[num_args][i] == '$' && args[num_args][i - 1] == '$'){
                args[num_args][i] = '\0';
				args[num_args][i - 1] = '\0';
				//expand to add in the pid
				snprintf(temp, max_line_length, "%s%d", args[num_args], getpid());
                //I tried doing this with sprintf and couldn't get it to work, so I used https://cplusplus.com/reference/cstdio/snprintf/
                //to help me with the syntax cause it was similar.
				args[num_args] = temp;
            }
            
        }
        token = strtok(NULL, " ");
        num_args++;
    }


    return num_args; //return to main so we know how many commands each line has
}

void check_args(int num_args){
    char currDir[128];
    int error_num;	
    //okay so first i need to check if the first character is a comment or a new line to ignore, then  check the first args
    //and do whatever needs to be done with them.
    if(args[0][0] == '#'){
        //do nothing, skip this whole function
    }
    else if(args[0][0] == '\n'){
        //also do nothing
    }
    //strcmp syntax from https://www.geeksforgeeks.org/strcmp-in-c-cpp/ cause I keep forgetting the == 0 at the end
    else if(strcmp(args[0], "exit") == 0){
        exit_shell();
    }
    else if(strcmp(args[0], "cd") == 0){
        if(num_args == 1){
            chdir(getenv("HOME"));
        }
        else{
            chdir(args[1]); //change to the 2nd arg ie cd CS162. also works with .. to go back just one cause it recognizes that
            //automatically
        }
        printf("%s\n", getcwd(currDir, 128));
        fflush(stdout);
    }
    else if(strcmp(args[0], "status") == 0){
        run_status(&error_num);
    }
    else{
        //Whenever a non-built in command is received, the parent (i.e., smallsh) will fork off a child.
        //The child will use a function from the exec() family of functions to run the command.
        //figure out later
        //for now just say none exists
        //this is where you'll want to start forking to run the function already in there
        // printf("%s: No such file or directory\n", args[0]);
        // fflush(stdout);
        execute_command(num_args, &error_num);
    }
}

void exit_shell(){
    if(total_processes == 0){
        exit(0);
    }
    //Exit syntax about what number to send from https://www.tutorialspoint.com/c_standard_library/c_function_exit.htm
    //with no processes we can just exit with 0 without killing any processes.
    //set up later, but if there are processes, kill off each one in the array
}

void run_status(int *error_num) {
    int exit_status = 0;
    int signal_number = 0;
    int exit_value = 0;

    int status;
    waitpid(getpid(), &status, 0);

    if (WIFEXITED(status)) {
        exit_status = WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        signal_number = WTERMSIG(status);
    }

    if (exit_status + signal_number == 0) {
        exit_value = 0;
    } else {
        exit_value = 1;
    }

    if (signal_number == 0) {
        printf("Exit value: %d\n", exit_value);
    } else {
        *error_num = 1;
        printf("Terminated by signal: %d\n", signal_number);
    }
    fflush(stdout);
}


char* searchInPath(char* cmd) {
    char* path = getenv("PATH");
    char* path_tok = strtok(path, ":");
    char cmd_path[1024];

    while(path_tok != NULL) {
        snprintf(cmd_path, sizeof(cmd_path), "%s/%s", path_tok, cmd);
        if(access(cmd_path, F_OK) == 0) {
            return strdup(cmd_path);
        }
        path_tok = strtok(NULL, ":");
    }

    return NULL;
}

void execute_command(int num_args, int* error_num) {
    pid_t pid;
    char* cmd;

    // Check if the command is to be run in the background
    if (strcmp(args[num_args - 1], "&") == 0) {
        // Only allow background execution if not in foreground only mode
        if (foreground_only == 0) {
            is_background = 1;
        }
        // Remove the '&' from the argument list
        args[num_args - 1] = NULL;
    }

    pid = fork();
    switch (pid) {
        case -1: // Error
            perror("Failed to create child process\n");
            exit(1);
            break;

        case 0: //child case
            //search for the command in the PATH
            cmd = searchInPath(args[0]);
            //if the command was found, execute it using execvp
            if (cmd != NULL) {
                execvp(cmd, args);
                //I'm assuming execvp is fine, it works soooo.
            } else {
                printf("%s: Command not found\n", args[0]);
                fflush(stdout);
                exit(1);
            }
            break;

        default: //parent case
            // If the command is not to be run in the background, wait for it to finish
            if (!is_background) {
                waitpid(pid, &status, 0);
                // run_status(error_num);
            } else {
                // Otherwise, print a message indicating it's running in the background
                printf("Background process %d created\n", pid);
                fflush(stdout);
            }
            break;
    }
}




//from assignment The exit command exits your shell. It takes no arguments. When this command is run, 
//your shell must kill any other processes or jobs that your shell has started before it terminates itself.
//probably gonna need an array of the processes, a count, and then kill them off 1 by one.