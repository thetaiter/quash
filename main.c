#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

int main(int argc, char **argv, char **envp) {
    // Initialize Variables
    char *input,  prompt[128];
    char *user = getenv("USER");
    char *home = getenv("HOME");
    char *path = getenv("PATH");

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

                // Get the first token in the string, delimiting at spaces and = signs
                char *token = strtok(input, " =");
            
                // If the first token is equal to "exit" or "quit"
                if (strcmp("exit", token) == 0 || strcmp("quit", token) == 0) {
                    // Break out of the infinite loop and terminal the program
                    break;
                }


            }
        }
    }
}
