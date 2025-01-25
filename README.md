# Mini-Shell

Welcome to **Mini-Shell**, a lightweight shell application implemented in C. This shell mimics basic Linux shell functionalities with support for command execution, I/O redirection, piping, and several built-in commands.

## Features

### General Command Execution
- Execute standard Linux commands and utilities by entering them directly.
- Handles command-line arguments seamlessly.

### Built-In Commands
- `cd [path]`: Change the current working directory to the specified `path`. Defaults to the home directory if no path is specified.
- `help`: Displays a list of available built-in commands and their usage.
- `prev`: Re-executes the last entered command.
- `source [file] [args...]`: Executes a script file with optional arguments.

### Input/Output Redirection
- Input redirection (`<`): Read input for a command from a specified file.
- Output redirection (`>`): Write the output of a command to a specified file, overwriting its contents.

### Piping
- Use `|` to pipe the output of one command as the input to another. For example:
  ```bash
  ls | grep .c
  ```
## To Run:
```
make shell
./shell
```
