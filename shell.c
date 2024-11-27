/* enhanced_shell.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#define INPUT_BUFFER_SIZE 1024
#define MAX_ARGUMENTS 64

extern char **environ; // Provides access to system-wide environment variables

// Function declarations
void process_command(char *command);
void navigate_to_directory(char *path);
void show_current_directory();
void perform_echo(char **args);
void update_environment_variable(char *name, char *value);
void display_environment_variables(char **args);
void handle_sigint_interrupt(int sig);
void handle_process_termination(int sig);
int handle_redirection_and_piping(char **args);

// Entry point of the shell program
int main() {
    char input[INPUT_BUFFER_SIZE];
    char working_directory[1024];

    // Setup handlers for interrupt and timeout signals
    signal(SIGINT, handle_sigint_interrupt);
    signal(SIGALRM, handle_process_termination);

    while (1) {
        // Display the shell prompt with the current directory
        if (getcwd(working_directory, sizeof(working_directory)) != NULL) {
            printf("%s> ", working_directory);
        }

        // Read input from the user
        if (fgets(input, INPUT_BUFFER_SIZE, stdin) == NULL) {
            break; // Exit on EOF or input error
        }

        // Remove trailing newline character
        input[strcspn(input, "\n")] = '\0';

        // Process the input command
        process_command(input);
    }

    return 0;
}

// Breaks down and processes the user's command
void process_command(char *command) {
    char *args[MAX_ARGUMENTS];
    char *token = strtok(command, " ");
    int arg_count = 0;

    // Parse the command into arguments
    while (token != NULL && arg_count < MAX_ARGUMENTS - 1) {
        args[arg_count++] = token;
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL; // Mark the end of the arguments array

    if (args[0] == NULL) {
        return; // Ignore empty commands
    }

    // Check for built-in commands
    if (strcmp(args[0], "cd") == 0) {
        navigate_to_directory(args[1]);
    } else if (strcmp(args[0], "pwd") == 0) {
        show_current_directory();
    } else if (strcmp(args[0], "echo") == 0) {
        perform_echo(args);
    } else if (strcmp(args[0], "env") == 0) {
        display_environment_variables(args);
    } else if (strcmp(args[0], "setenv") == 0 && args[1] && args[2]) {
        update_environment_variable(args[1], args[2]);
    } else if (strcmp(args[0], "exit") == 0) {
        exit(0); // Exit the shell program
    } else {
        // Handle redirection and piping
        if (handle_redirection_and_piping(args) == 0) {
            return;
        }

        // Execute external commands
        pid_t pid = fork();
        if (pid == 0) { // Child process
            alarm(10); // Limit process execution time to 10 seconds
            if (execvp(args[0], args) < 0) {
                perror("Error executing command");
                exit(1);
            }
        } else if (pid > 0) { // Parent process
            waitpid(pid, NULL, 0); // Wait for the child process to complete
            alarm(0); // Clear the alarm after process finishes
        } else {
            perror("Failed to create process");
        }
    }
}

// Change the current working directory
void navigate_to_directory(char *path) {
    if (chdir(path) != 0) {
        perror("Error changing directory");
    }
}

// Display the current directory
void show_current_directory() {
    char directory[1024];
    if (getcwd(directory, sizeof(directory)) != NULL) {
        printf("%s\n", directory);
    } else {
        perror("Error retrieving current directory");
    }
}

// Display text or environment variable values
void perform_echo(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        if (args[i][0] == '$') { // Check for environment variable
            char *value = getenv(args[i] + 1);
            if (value) {
                printf("%s ", value);
            }
        } else {
            printf("%s ", args[i]);
        }
    }
    printf("\n");
}

// Update or create an environment variable
void update_environment_variable(char *name, char *value) {
    if (setenv(name, value, 1) != 0) {
        perror("Error setting environment variable");
    }
}

// Display all environment variables or a specific one
void display_environment_variables(char **args) {
    if (args[1]) { // Display a specific variable
        char *value = getenv(args[1]);
        if (value) {
            printf("%s\n", value);
        } else {
            printf("Environment variable '%s' not found\n", args[1]);
        }
    } else { // Display all environment variables
        for (char **env = environ; *env != NULL; env++) {
            printf("%s\n", *env);
        }
    }
}

// Handle SIGINT interruptions (Ctrl+C)
void handle_sigint_interrupt(int sig) {
    printf("\nSignal %d received. Type 'exit' to close the shell.\n", sig);
}

// Handle process timeouts using SIGALRM
void handle_process_termination(int sig) {
    printf("\nProcess timeout exceeded. Terminating the process.\n");
    kill(0, SIGKILL);
}

// Handle input/output redirection and piping
int handle_redirection_and_piping(char **args) {
    int pipe_fds[2];
    int has_pipe = 0;

    // Detect piping
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            has_pipe = 1;
            args[i] = NULL; // Split command at pipe
            pipe(pipe_fds);
        }
    }

    if (has_pipe) {
        pid_t pid1 = fork();
        if (pid1 == 0) { // First process in pipeline
            dup2(pipe_fds[1], STDOUT_FILENO); // Redirect stdout to pipe
            close(pipe_fds[0]);
            close(pipe_fds[1]);
            execvp(args[0], args); // Execute first command
            perror("Pipeline execution error");
            exit(1);
        }

        pid_t pid2 = fork();
        if (pid2 == 0) { // Second process in pipeline
            dup2(pipe_fds[0], STDIN_FILENO); // Redirect stdin from pipe
            close(pipe_fds[0]);
            close(pipe_fds[1]);
            execvp(args[1], &args[1]); // Execute second command
            perror("Pipeline execution error");
            exit(1);
        }

        close(pipe_fds[0]);
        close(pipe_fds[1]);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        return 0;
    }

    // Handle redirection
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            int output_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd < 0) {
                perror("Error opening file for output redirection");
                return 0;
            }
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
            args[i] = NULL;
            break;
        } else if (strcmp(args[i], "<") == 0) {
            int input_fd = open(args[i + 1], O_RDONLY);
            if (input_fd < 0) {
                perror("Error opening file for input redirection");
                return 0;
            }
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
            args[i] = NULL;
            break;
        }
    }

    return 1;
}
