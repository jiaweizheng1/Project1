#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h> // To access waitpid function

#define CMDLINE_MAX 512

char *parseargsfunction(char *process, int *args_counter);
char *removeleadspaces(char *str, int *spacesremoved);

int main(void)
{
        char cmd[CMDLINE_MAX];

        while (1)
        {
                char *nl;
                //int retval;

                // Print prompt
                printf("sshell$ ");
                fflush(stdout);

                // Get command line
                fgets(cmd, CMDLINE_MAX, stdin);

                // Print command line if stdin is not provided by terminal
                if (!isatty(STDIN_FILENO))
                {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                // Remove trailing newline from command line
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';

                // Get a copy of cmd for parsing
                char *cmd_cpy;
                int num_spaces = 0;
                cmd_cpy = removeleadspaces(cmd, &num_spaces);
                printf("%s\n", cmd_cpy);

                // Builtin command exit
                if (!strcmp(cmd, "exit"))
                {
                        fprintf(stderr, "Bye...\n");
                        break;
                }

                // Built-in command pwd
                // with help from https://www.gnu.org/software/libc/manual/html_mono/libc.html#Working-Directory
                if (!strcmp(cmd, "pwd"))
                {
                        char file_name_buf[_PC_PATH_MAX];
                        getcwd(file_name_buf, sizeof(file_name_buf)); // Prints out filename representing current directory.
                }

                // Start of process parsing
                // with help from https://www.codingame.com/playgrounds/14213/how-to-play-with-strings-in-c/string-split
                // with help from https://stackoverflow.com/questions/12460264/c-determining-which-delimiter-used-strtok
                char *processes[5];       // up to 3 pipe signs or 4 processes + 1 output redirection; only 1 process can be up to 512 characters; dynamic 2d ptr array
                char process_seps[4][3];  // up to 3 pipe signs + 1 output redirection sign; 2d array
                char delimiter[] = ">|&"; // We want to parse the string, ignoring all spaces, >, and |
                int num_processes = 0;    // Integer for selecting indexes in processes array
                int num_process_seps = 0; // Integer for selecting indexes in process separators array
                int index_output_redirec = -1;
                int expecting_one_file_arg = 0;
                char *ptr;
                if (cmd_cpy[0] == '>')
                {
                        if (cmd_cpy[1] == '&')
                        {
                                strcpy(process_seps[num_process_seps], ">&");
                        }
                        else
                        {
                                strcpy(process_seps[num_process_seps], ">");
                        }
                        num_process_seps++;
                        index_output_redirec = num_process_seps;
                        expecting_one_file_arg = 1;
                }
                else if (cmd_cpy[0] == '|')
                {
                        if (cmd_cpy[1] == '&')
                        {
                                strcpy(process_seps[num_process_seps], "|&");
                        }
                        else
                        {
                                strcpy(process_seps[num_process_seps], "|");
                        }
                        num_process_seps++;
                }
                ptr = strtok(cmd_cpy, delimiter); // ptr points to each process in the string
                while (ptr != NULL)               // keep parsing until end of user input
                {
                        processes[num_processes] = ptr;                           // Copy each process of cmd to args array
                        if (cmd[num_spaces + ptr - cmd_cpy + strlen(ptr)] == '>') // <------ I dont understand this part at all
                        {
                                if (cmd[num_spaces + ptr - cmd_cpy + strlen(ptr) + 1] == '&')
                                {
                                        strcpy(process_seps[num_process_seps], ">&");
                                }
                                else
                                {
                                        strcpy(process_seps[num_process_seps], ">");
                                }
                                num_process_seps++;
                                index_output_redirec = num_process_seps;
                                expecting_one_file_arg = 1;
                        }
                        else if (cmd[num_spaces + ptr - cmd_cpy + strlen(ptr)] == '|')
                        {
                                if (cmd[num_spaces + ptr - cmd_cpy + strlen(ptr) + 1] == '&')
                                {
                                        strcpy(process_seps[num_process_seps], "|&");
                                }
                                else
                                {
                                        strcpy(process_seps[num_process_seps], "|");
                                }
                                num_process_seps++;
                                expecting_one_file_arg = 0;
                        }
                        else
                        {
                                expecting_one_file_arg = 0;
                        }
                        ptr = strtok(NULL, delimiter);
                        num_processes++;
                }
                for (int x = 0; x < num_processes; x++)
                {
                        printf("%s\n", processes[x]);
                }
                for (int x = 0; x < num_process_seps; x++)
                {
                        printf("%s\n", process_seps[x]);
                }
                printf("%d\n", num_processes);
                printf("%d\n", num_process_seps);
                // End of process parsing

                // Some error checking
                if (expecting_one_file_arg)
                {
                        fprintf(stderr, "Error: no output file\n");
                        continue;
                }
                else if (index_output_redirec != -1 && index_output_redirec < num_process_seps)
                {
                        fprintf(stderr, "Error: mislocated output redirection\n");
                        continue;
                }
                else if (cmd[0] != '\0' && num_process_seps >= num_processes)
                {
                        fprintf(stderr, "Error: missing command\n");
                        continue;
                }

                int num_args = 0;
                char *arg = (parseargsfunction(processes[0], &num_args));
                printf("%s\n", arg);
                /*
                printf("%d\n", num_args);
                for (int x = 0; x < num_args; x++)
                {
                        printf("%s\n", arg[x]);
                }
                */

                /*
                char *args[16] = {"cd", "lol"};
                // cd implementation
                if (!strcmp(args[0], "cd"))
                {
                        if (chdir(args[1]) < 0) // Set process's working directory to file name specified by 2nd argument
                        {
                                fprintf(stderr, "Error: cannot cd into directory\n"); // error if not successful
                                continue;
                        }
                        continue;
                }
                */

                /*
                // Regular command
                pid_t pid = fork(); // Fork, creating child process
                if (pid == 0)       // Child
                {
                        if (execvp(args[0], args) < 0) // Set process's working directory to file name specified by 2nd argument
                        {
                                fprintf(stderr, "Error: command not found\n"); // error if not successful
                                continue;
                        }
                        exit(0); // try to exit with exit successful flag
                }
                else if (pid != 0) // Parent
                {
                        int child_status;
                        waitpid(pid, &child_status, 0);     // Wait for child to finishing executing, then parent continue operation
                        retval = WEXITSTATUS(child_status); // WEXITSTATUS gets exit status of child and puts it into retval
                }
                fprintf(stderr, "Return status value for '%s': %d\n", cmd, retval); // Prints exit status of child
                */
        }

        return EXIT_SUCCESS;
}

char *parseargsfunction(char *process, int *args_counter)
{

        char *args[16];
        int index_args = 0;
        char delimiter[] = " ";
        char *ptr = strtok(process, delimiter);
        while (ptr != NULL)
        {
                args[index_args] = ptr;
                index_args++;
                ptr = strtok(NULL, delimiter);
        }
        for (int x = 0; x < index_args; x++)
        {
                printf("%s\n", args[x]);
        }
        *args_counter += index_args;
        return *args;
}

// with help from https://www.geeksforgeeks.org/c-program-to-trim-leading-white-spaces-from-string/
char *removeleadspaces(char *str, int *spacesremoved)
{
        static char ret_str[CMDLINE_MAX];
        int count = 0;
        int i;
        int j;
        while (str[count] == ' ') // iterate string until last leading space char
        {
                count++;
        }

        for (i = count, j = 0; str[i] != '\0'; i++, j++) // copy input string(without leading spaces) to return string
        {
                ret_str[j] = str[i];
        }
        ret_str[j] = '\0'; // add newline on last index to signify end of string
        *spacesremoved += count;
        return ret_str;
}

/*
if (i > 15)
{
        fprintf(stderr, "Error: too many process arguments\n"); // error if too many arguments passed in through terminal
        break;
}
*/