# Quash - A Simple Command Shell

### Course: Operating Systems CSCI-401 
### Student Name(s): Caitlyn Lynch (@02995610), Kristian Morgan (@02937236)

---

## Introduction

**Quash** is a simple command-line shell designed to mimic the functionality of widely used shells like bash and sh. This project provides an opportunity to understand core concepts like process forking, signaling, I/O redirection, and piping while building a functional shell interface.  

The shell supports built-in commands, external command execution, background processes, and additional features like redirection and piping. This document outlines the design, implementation details, and testing methodology used to verify the functionality of the shell.

---

## Design Choices and Testing Methodology

### Built-in Commands
The following commands are implemented internally for efficiency and direct control over shell behavior:

- **`cd`**: Change directory. Tested with valid and invalid paths:
  ```bash
  cd ..
  cd nonexistent
  cd
  ```

- **`pwd`**: Display the current working directory. Verified the output after `cd`:
  ```bash
  pwd
  ```

- **`echo`**: Print arguments or environment variable values. Tested with strings and variables:
  ```bash
  echo "Hello, World!"
  echo $PATH
  echo $INVALID_VAR
  ```

- **`env`**: List environment variables or display a single variable. Verified handling of valid and invalid inputs:
  ```bash
  env
  env PATH
  env NONEXISTENT_VAR
  ```

- **`setenv`**: Set or update environment variables. Verified correct addition or modification:
  ```bash
  setenv TEST_VAR 123
  echo $TEST_VAR
  env | grep TEST_VAR
  ```

- **`exit`**: Exit the shell. Tested clean termination:
  ```bash
  exit
  ```

### External Commands
Quash uses `fork()`, `execvp()`, and `waitpid()` to execute external commands like `ls`, `cat`, and `grep`. Valid and invalid commands were tested:
```bash
ls
cat shell.c
nonexistent_command
```

### Background Processes
Commands ending with `&` run in the background. Verified process responsiveness:
```bash
sleep 5 &
ps
```

### Signal Handling
Custom handlers manage the following:

- **SIGINT**: Prevents shell termination during foreground process interruptions:
  ```bash
  sleep 20
  # Press Ctrl+C to verify shell remains active
  ```

- **SIGALRM**: Automatically terminates processes exceeding a 10-second runtime:
  ```bash
  sleep 20
  ```

### I/O Redirection
Quash supports input and output redirection:

- **Output (`>`):** Redirects standard output to a file. Verified file content:
  ```bash
  ls > output.txt
  cat output.txt
  ```

- **Input (`<`):** Redirects standard input from a file. Verified correct behavior:
  ```bash
  echo "Hello there" > input.txt
  cat < input.txt
  ```

### Piping
Commands can be connected with pipes (`|`). Single and chained pipes were tested:
```bash
ls | grep shell
cat input.txt | grep "Hello" | wc -l
```

---

## Key Features
- **Prompt Display**: Displays the current working directory.
- **Built-in Commands**: Includes `cd`, `pwd`, `echo`, `env`, `setenv`, and `exit`.
- **External Commands**: Executes programs and gracefully handles invalid commands.
- **Background Processes**: Supports execution with `&`.
- **Signal Handling**: Manages `SIGINT` and `SIGALRM`.
- **I/O Redirection**: Supports `>` and `<`.
- **Piping**: Connects commands using `|`.

---

## Compilation and Execution

### Compilation
Use the provided Makefile to compile Quash:
```bash
make
```

### Running the Shell
To start Quash:
```bash
./shell
```

### Clean Up
Remove build artifacts with:
```bash
make clean
```

---

## Conclusion

The Quash shell is a culmination of key concepts in operating systems and software design, showcasing a strong understanding of shell mechanics and process management. By integrating essential shell functionalities, including built-in commands, external command execution, background processes, signal handling, I/O redirection, and piping, Quash offers a versatile and reliable command-line interface. 

Through rigorous testing, the shell has demonstrated robust performance under various scenarios, ensuring its ability to handle both expected use cases and edge cases. Each feature was carefully implemented with attention to detail, ensuring a seamless user experience and adherence to real-world shell behavior.

The project not only reinforced fundamental concepts like process forking, signaling, and inter-process communication but also provided valuable experience in debugging and optimizing complex systems. The Quash shell serves as a practical example of how theoretical knowledge can be translated into a functional and meaningful application, emphasizing its relevance in both academic and professional contexts.

This project is a testament to the importance of problem-solving, collaboration, and iterative development in creating effective software solutions. Quash is a significant milestone in understanding the inner workings of operating systems and stands as a foundation for further exploration into more advanced system-level programming and operating systems challenges.

---

## Acknowledgments
Special thanks to Professor Lee Burge, Ph.D. and Howard University College of Engineering and Architecture for their guidance and support.
