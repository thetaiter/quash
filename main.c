#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <readline/readline.h>

char *trimWhiteSpaces(char *str) {
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

char **tokenize(char* input) {
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

		temp = strtok(NULL, " =");
	}

	ret = realloc(ret, sizeof(char*) * (numSpaces+1));
	ret[numSpaces] = 0;

	return ret;
}

void handleSigInt(int sig) {
	printf("\n\nSIGINT signal %i received. Quitting quash.\n\n", sig);
	exit(0);
}

int main(int argc, char **argv, char **envp) {
    // Initialize Variables
    char *input, prompt[128];
    char *user = getenv("USER");
    char *home = getenv("HOME");
    char *path = getenv("PATH");

    // Check if SIGINT signal is received and set handler
    signal(SIGINT, handleSigInt);

    // Ininite Loop
    while(1) {
        // Setup the quash's prompt
        snprintf(prompt, sizeof(prompt), "%s : %s > ", user, getcwd(NULL, 1024));
        
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
                char **tokens = tokenize(input);

                // If the first token is equal to "exit" or "quit"
                if (strcmp("exit", tokens[0]) == 0 || strcmp("quit", tokens[0]) == 0) {
                    // Break out of the infinite loop and terminate the program
                    break;
                }

                // If the first token is 'set'
                if (strcmp("set", tokens[0]) == 0) {
                	// Get the variable to be set and the value to set it to
                	char *newVar = tokens[1];
                	char *value = tokens[2];

            		setenv(newVar, value, 0);

            		if (strcmp(value, getenv(newVar)) == 0) {
            			printf("\nNew environment variable '%s' was set to the value '%s' successfully.\n\n", newVar, getenv(newVar));
            		} else {
            			printf("\nValue for '%s' was already set to '%s'.\nWould you like to reset this environment variable to '%s'?", newVar, getenv(newVar), value);
            			char *temp = readline(" (y/n): ");

            			if (strcasecmp(temp, "y") == 0 || strcasecmp(temp, "yes") == 0) {
            				setenv(newVar, value, 1);
            				printf("Environment variable '%s' was set to the value '%s' successfully.\n\n", newVar, getenv(newVar));
            			} else {
            				printf("%s", "\n");
         					}
            		}
                }
            }
        }
    }

    printf("%s", "\nThank you for using this quash program. Have a wonderful day!\n\n");
}