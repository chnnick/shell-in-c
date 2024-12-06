#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>  // For file I/O operations

#define MAX_INPUT_SIZE 256
#define MAX_NUM_TOKENS 160

// Function to read input from user
char *read_input() {
    static char input[MAX_INPUT_SIZE]; // Allocate input buffer
    if (fgets(input, sizeof(input), stdin) == NULL) {
        if (feof(stdin)) { // Handle Ctrl-D (EOF)
            printf("\nBye bye.\n");
            exit(0);
        } else {
            perror("fgets failed");
            exit(1);
        }
    }
    // *** Remove the newline character if present ***
    size_t len = strlen(input);
    if (input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }
    return input;
}

/**
  Tells you if the input character is a special ()<>;| character.
*/
int isSpecialCharacter(char input) {
    return (input == '(' || input == ')' || input == '>' || input == '<' || input == ';' || input == '|');
}

/**
  Reads input until delimiter found OR end of string
  input will be placed in output variable.
  Disregards special characters while in ".
*/
int readQuotedNextInput(char *input, char *output) {
    int outputLength = 0;

    while (input[outputLength] != '"' && input[outputLength] != '\0') {
        output[outputLength] = input[outputLength];
        outputLength++;
    }
    output[outputLength] = '\0'; // add terminating character at end after delimiter found / '\0'
    return outputLength;
}

/**
  Reads input until delimiter found OR end of string
  input will be placed in output variable.
*/
int readNextInput(char *input, char *output) {
    int outputLength = 0;

    while (!isSpecialCharacter(input[outputLength]) && input[outputLength] != ' ' && input[outputLength] != '\0') {
        output[outputLength] = input[outputLength];
        outputLength++;
    }
    output[outputLength] = '\0'; // add terminating character at end after delimiter found / '\0'
    return outputLength;
}

// adds token to tokens string array.
void addToken(char *buffer, char **tokens, int tokenId) {
    // Allocate memory for the token, including space for the null terminator
    tokens[tokenId] = malloc(strlen(buffer) + 1);
    // Copy the contents of buffer into the newly allocated memory
    strcpy(tokens[tokenId], buffer);
}

// frees tokens array.
void freeTokens(char **tokens) {
    int i = 0;
    while (tokens[i] != NULL) {
        free(tokens[i]);
        i++;
    }
    free(tokens);
}

// should return pointer to pointer of a char
char **tokenize(char *input) {
    char **tokens = malloc(MAX_NUM_TOKENS * sizeof(char *));
    char buf[MAX_INPUT_SIZE];
    // index of the input string
    int i = 0;
    int tokenId = 0;
    while (input[i] != '\0') {
        // Check for quotes
        if (input[i] == '"') {
            ++i; // do not want to put beginning '"' into buf
            i += readQuotedNextInput(&input[i], buf);
            addToken(buf, tokens, tokenId);
            ++tokenId;
            ++i; // do not want to put ending '"' into buf
            continue; // skip the rest of this iteration
        }
        // Check for special character
        else if (isSpecialCharacter(input[i])) {
            // Add single-character token including this input to the tokens list.
            buf[0] = input[i];
            buf[1] = '\0';
            addToken(buf, tokens, tokenId);
            ++tokenId;
            ++i;
        }
        // Check for spaces
        else if (input[i] == ' ') {
            ++i; // skip past spaces
        }
        else {
            i += readNextInput(&input[i], buf);
            addToken(buf, tokens, tokenId);
            ++tokenId;
            continue;
        }
    }
    tokens[tokenId] = NULL; // terminate end of tokens array.
    return tokens;
}

// Function to handle input redirection (<)
void handle_input_redirection(char *file) {
    int fd = open(file, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open input file");
        exit(1);
    }
    dup2(fd, STDIN_FILENO); // Replace standard input with file descriptor
    close(fd);
}

// Function to handle output redirection (>)
void handle_output_redirection(char *file) {
    int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Failed to open output file");
        exit(1);
    }
    dup2(fd, STDOUT_FILENO); // Replace standard output with file descriptor
    close(fd);
}

// Function to handle pipes (|)
// This function creates a pipe and connects the output of the left-hand command to the input of the right-hand command
void handle_pipe(char **lhs_tokens, char **rhs_tokens) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(1);
    }

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid1 == 0) { // Child process 1
        close(pipefd[0]);              // Close unused read end of the pipe
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to the pipe
        close(pipefd[1]);              // Close the write end of the pipe
        execvp(lhs_tokens[0], lhs_tokens);
        perror(lhs_tokens[0]); // In case exec fails
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid2 == 0) { // Child process 2
        close(pipefd[1]);              // Close unused write end of the pipe
        dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to the pipe
        close(pipefd[0]);              // Close the read end of the pipe
        execvp(rhs_tokens[0], rhs_tokens);
        perror(rhs_tokens[0]); // In case exec fails
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]); // Close both ends of the pipe in the parent process
    close(pipefd[1]);
    waitpid(pid1, NULL, 0); // Wait for both child processes to finish
    waitpid(pid2, NULL, 0);
}

// Function to execute the command with possible redirection
void execute_command(char **tokens) {
    int input_redirection = -1, output_redirection = -1;
    char *input_file = NULL, *output_file = NULL;
    int i = 0;

    // Look for input/output redirection symbols
    while (tokens[i] != NULL) {
        if (strcmp(tokens[i], "<") == 0) {
            input_redirection = i;
            input_file = tokens[i + 1];
            tokens[i] = NULL; // Remove "<" from tokens
        } else if (strcmp(tokens[i], ">") == 0) {
            output_redirection = i;
            output_file = tokens[i + 1];
            tokens[i] = NULL; // Remove ">" from tokens
        }
        i++;
    }

    pid_t pid = fork(); // Fork a child process
    if (pid < 0) {
        perror("fork failed");
    } else if (pid == 0) {
        // In the child process
        if (input_redirection != -1) {
            handle_input_redirection(input_file); // Handle input redirection
        }
        if (output_redirection != -1) {
            handle_output_redirection(output_file); // Handle output redirection
        }
        execvp(tokens[0], tokens);
        perror(tokens[0]); // Print error message if exec fails
        exit(EXIT_FAILURE);
    } else {
        // In the parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to finish
    }
}

void helpMessage() {
    printf("---- LIST OF BUILT-IN FUNCTIONS ----\n");
    printf("cd (Change Directory):\n");
    printf("- cd [path]\n");
    printf("- Change current directory to PATH specified in the argument\n");
    printf("source:\n");
    printf("- source filename [arguments]\n");
    printf("- Read and execute commands from FILENAME and return.\n");
    printf("prev:\n");
    printf("- Prints the previous command line and executes it again,\n");
    printf("- without becoming the new command line.\n");
    printf("help:\n");
    printf("- Shows this help message.\n");
}

/**
  Runs a source command, assuming that given token input[0] is source.
*/
void runSourceCommand(char **tokens) {
  // read second element, if empty, there is no file, thus return.
  if (tokens[1] == NULL) {
    printf("No source file found.\n");
    return;
  }
  // Count number of arguments (including script file)
  int numArgs = 0;
  while (tokens[numArgs] != NULL) {
    numArgs++;
  }
  char **sourceArgs = malloc((numArgs) * sizeof(char *));
  // Change permissions to make the script executable
  if (chmod(tokens[1], 0777) == -1) {
      printf("Error changing file permissions");
      return;
  }
  // Copy tokens to sourceArgs (start at index 0 for the program name)
  for (int i = 1; i < numArgs; ++i) {
    sourceArgs[i - 1] = tokens[i];
  }
  // terminate the end of the sourceArgs array.
  sourceArgs[numArgs - 1] = NULL;
  // Execute the source script with arguments
  if (execvp(tokens[1], sourceArgs) == -1) {
    printf("Error executing source command");
  }
  // free arguments
  freeTokens(sourceArgs);
}


// Process individual commands
int processCommand(char **tokens, char **prevTokens) {
    if (tokens == NULL || tokens[0] == NULL) {
        return 1;
    }

    // Check for built-in command 'exit'
    if (strcmp(tokens[0], "exit") == 0) {
        printf("Bye bye.\n");
        return 0;
    }
    // Check for built-in command 'cd'
    else if (strcmp(tokens[0], "cd") == 0) {
        if (tokens[1] == NULL) {
            chdir(getenv("HOME"));
        } else if (chdir(tokens[1]) == -1) {
            perror("No directory found");
        }
    }
    // Check for built-in command 'source'
    else if (strcmp(tokens[0], "source") == 0) {
        runSourceCommand(tokens);
    }
    // Check for built-in command 'help'
    else if (strcmp(tokens[0], "prev") == 0) {
      if (prevTokens == NULL) {
        printf("No previous commands\n");
      } else {
        execute_command(prevTokens);
      }
    }
    // Check for built-in command 'help'
    else if (strcmp(tokens[0], "help") == 0) {
        helpMessage();
    }
    // Handle non-built-in commands
    else {
        execute_command(tokens);
    }
    return 1;
}

/**
  Handles commands, first checking if it is a sequence requiring | or ;, then running those individual processes.
*/
int processSequence(char *input, char **prevTokens) {
  char **tokens = tokenize(input);  // Tokenize the input
  int shouldContinue = 1;
  int i = 0;

  // Handle sequences separated by ;
  while (tokens[i] != NULL) {
    if (strcmp(tokens[i], ";") == 0) {
      tokens[i] = NULL;  // Split the command before ";"

      // Process the command before ";"
      shouldContinue = processCommand(tokens, prevTokens);  
      
      // Move to the next command after ";"
      tokens = &tokens[i + 1];
      i = -1;  // Reset the index to start from the beginning of the new command
    } else if (strcmp(tokens[i], "|") == 0) {
      tokens[i] = NULL;  // Split the tokens at "|"

      // Handle piping between commands
      handle_pipe(tokens, &tokens[i + 1]);  
      return 1;  // Return after handling the pipe
    }
    i++;
  }

  // Process the remaining tokens after the last semicolon
  shouldContinue = processCommand(tokens, prevTokens);

  return shouldContinue;
}

int main(int argc, char **argv) {
  char *input;
  char **prevTokens = NULL;
  // Print welcome message
  printf("Welcome to mini-shell.\n");

  int shouldContinue = 1;
  // Main loop for the shell
  while (shouldContinue) {
    printf("shell $ ");
    // Read input
    input = read_input();
    // Process sequence of commands
    shouldContinue = processSequence(input, prevTokens);
    free(prevTokens);
    prevTokens = tokenize(input);
  }

  return 0;
}

