#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <readline/readline.h>

struct job {
    int jid;
    int pid;
    char *com;
};

static int *numJobs;
static struct job jobs[20];

void handleSigInt(int sig) {
	printf("\n\nSIGINT signal %i received. Quitting quash.\n\n", sig);
	exit(0);
}

char* trimWhiteSpaces(char *str) {
    // Move string pointer to first character that is not a space
    while(isspace(*str)) {
        str++;
    }

    // If the string was all spaces, return the null string
    if (*str == 0) {
        return str;
    }

    // Get a pointer to the last character in the string
    char *lastChar = str + strlen(str) - 1;

    // While the last character pointer is not at the first character of the string,
    // and while it is a space, move the last character pointer back one character
    while(lastChar > str && isspace(*lastChar)) {
        lastChar--;
    }
    
    // Set the new last character to the null character
    *(lastChar+1) = 0;

    // Return the new string with no spaces at the beginning or end
    return str;
}

char** tokenize(char *input, int* numArgs) {
	char **ret = NULL;
	char *temp = strtok(input, " ");
	int i, numSpaces = 0;

	while(temp) {
		ret = realloc(ret, sizeof(char*) * ++numSpaces);

		if (ret == NULL) {
			printf("\nMemory allocation failed for command '%s'\n\n", input);
			return ret;
		}

		// If token is a string, put entire string in token and remove quotes
		if (temp[0] == '"') {
			temp++;
			sprintf(temp, "%s%s%s", temp, " ", strtok(NULL, "\""));
		}

		ret[numSpaces-1] = temp;
		*numArgs = (*numArgs) + 1;

		temp = strtok(NULL, " =");
	}

	ret = realloc(ret, sizeof(char*) * (numSpaces+1));
	ret[numSpaces] = 0;

	return ret;
}

void set(char **args) {
    // Get the variable to be set and the value to set it to
    char *newVar = args[1];
    char *value = args[2];
    setenv(newVar, value, 0);

    if (strcmp(value, getenv(newVar)) == 0) {
        printf("\nNew environment variable '%s' was set to the value '%s' successfully.\n\n", newVar, getenv(newVar));
    } else {
   		printf("\nValue for '%s' is already set to '%s'.\nWould you like to reset this environment variable to '%s'?", newVar, getenv(newVar), value);
       	char *temp = readline(" (y/n): ");

        if (strcasecmp(temp, "y") == 0 || strcasecmp(temp, "yes") == 0) {
          	setenv(newVar, value, 1);
           	printf("Environment variable '%s' was set to the value '%s' successfully.\n\n", newVar, getenv(newVar));
        } else {
	           	printf("%s", "\n");
       	}
    }
}

int findStrPosition(char **strs, int numStrs, char* toFind) {
    int i;

    for (i = 0; i < numStrs; i++) {
        if (strcmp(strs[i], toFind) == 0) {
            return i;
        }
    }

    return -1;
}

void cd(char *path) {
    if (!path) {
        if (chdir(getenv("HOME")) == -1) {
            printf("\n'%s' in not a valid path.\n\n", strerror(errno));
        }
    } else {
    	if (chdir(path) == -1) {
            printf("\n'%s' in not a valid path.\n\n", strerror(errno));
    	}
    }
}

void printJobs() {
    int i;

    for (i = 0; i < *numJobs; i++) {
        if (kill(jobs[i].pid, 0) == 0) {
            printf("\nJob ID: %d\nPID: %d\nCommand: %s", jobs[i].jid, jobs[i].pid, jobs[i].com);
        }
    }
}

void pipes(char **args) {
	int pipefd[2];
	char *first_arg = args[0];
	char *second_arg = args[2];
	char *argv1[] = {first_arg, NULL};
	char *argv2[] = {second_arg, NULL};
	int pid;
    
		if (pipe(pipefd) == -1) {
			perror("pipe");
			exit(1);
		}
      
		pid_t pid_1, pid_2;
    
		pid_1 = fork();
		if (pid_1 == 0) {
			dup2(pipefd[1], STDOUT_FILENO);
			close(pipefd[0]);
			close(pipefd[1]);
				if (execvp(first_arg, argv1) == -1)
					perror("quash");
			//parser(trimWhiteSpaces(first_arg));
			exit(0);
		}
    
		pid_2 = fork();
		if (pid_2 == 0) {
			dup2(pipefd[0], STDIN_FILENO);
			close(pipefd[1]);
			close(pipefd[0]);
			if (execvp(second_arg, argv2) == -1)
				perror("quash");
			//parser(trimWhiteSpaces(second_arg));
			//exit(0);
		}
}

void executeExternalCommand(char **args) {

}

void executeCommand(char **args, int numArgs) {
    if (strcmp("set", args[0]) == 0) {
     	set(args);
    } else if (strcmp("cd", args[0]) == 0) {
    	cd(args[1]);
    } else if (strcmp("jobs", args[0]) == 0) {
    	printJobs();
    } else {
    
    }
}

int main(int argc, char *argv[], char *envp[]) {
    // Initialize Variables
    char *input, prompt[128];
    int numArgs = 0;

    // Check if SIGINT signal is received and set handler
    signal(SIGINT, handleSigInt);

    // Ininite Loop
    while(1) {
    	char *user = getenv("USER");
        char *home = getenv("HOME");
        char *path = getenv("PATH");
        char *cwd = getcwd(NULL, 1024);

        // Setup the quash's prompt
        snprintf(prompt, sizeof(prompt), "%s : %s > ", user, cwd);
        
        // Read the user's input, displaying the prompt
        input = readline(prompt);

        // If the input is not null, do things with it, else continue loop
        if (*input) {
            // Remove the white spaces at the beginning and end of the input
            input = trimWhiteSpaces(input);

            if (*input) {
                // Add the current input to the history
                add_history(input);

                // Tokenize input and place tokens into an array
                char **tokens = tokenize(input, &numArgs);

                // If the first token is equal to "exit" or "quit"
                if (strcmp("exit", tokens[0]) == 0 || strcmp("quit", tokens[0]) == 0) {
                    // Break out of the infinite loop and terminate the program
                    break;
                }

                executeCommand(tokens, numArgs);
            }
        }

        numArgs = 0;
    }

    printf("%s", "\nThank you for using this quash program. Have a wonderful day!\n\n");
}
