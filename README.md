# POSIX-Compliant Unix Shell

A fully-featured Unix shell implementation in C that adheres to POSIX standards, providing comprehensive command execution, job control, I/O redirection, and pipeline support. This project demonstrates deep understanding of Unix system programming, process management, and inter-process communication.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
- [Built-in Commands](#built-in-commands)
- [Advanced Features](#advanced-features)
- [Project Structure](#project-structure)
- [Technical Implementation](#technical-implementation)
- [Examples](#examples)
- [Testing](#testing)
- [Known Limitations](#known-limitations)
- [Contributing](#contributing)
- [License](#license)

## Overview

This project implements a production-quality Unix shell from scratch in C, following POSIX standards. The shell provides a command-line interface for interacting with the operating system, executing commands, managing processes, and controlling job execution. It serves as both a practical tool and an educational demonstration of Unix systems programming concepts.

## Features

### Core Functionality
- **Command Execution**: Support for both built-in and external commands
- **Pipeline Operations**: Multi-stage pipelines with arbitrary depth (`cmd1 | cmd2 | cmd3`)
- **I/O Redirection**: Input (`<`), output (`>`), and append (`>>`) redirection
- **Job Control**: Foreground and background process management
- **Signal Handling**: Proper handling of SIGINT, SIGTSTP, SIGTTIN, and SIGTTOU
- **Command History**: Persistent command logging and execution

### Process Management
- **Background Execution**: Run commands asynchronously with `&`
- **Job Monitoring**: Track and manage multiple background jobs
- **Process Groups**: Proper PGID management for job control
- **Signal Forwarding**: Send arbitrary signals to processes

### Built-in Commands
- **hop**: Enhanced directory navigation with history
- **reveal**: Directory listing with filtering options
- **log**: Command history management and execution
- **activities**: Display all running and stopped jobs
- **ping**: Send signals to processes
- **fg/bg**: Foreground and background job control

### Shell Features
- **Custom Prompt**: Dynamic prompt showing username, hostname, and current directory
- **Relative Path Display**: Smart display of paths relative to shell home
- **Command Chaining**: Sequential execution with semicolons (`;`)
- **Tab Completion Support**: Ready for tab completion extension
- **Error Handling**: Comprehensive error reporting and recovery

## Architecture

The shell is organized into modular components, each responsible for specific functionality:

```
┌─────────────────────────────────────────────────────────┐
│                    User Interface                        │
│              (Prompt & Input Handling)                   │
└───────────────────────┬─────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────┐
│                   Command Parser                         │
│     (Tokenization, Syntax Analysis, Tree Building)      │
└───────────────────────┬─────────────────────────────────┘
                        │
            ┌───────────┴───────────┐
            │                       │
            ▼                       ▼
┌────────────────────┐    ┌──────────────────┐
│  Built-in Commands │    │  External Cmds   │
│  (Intrinsics)      │    │  (Executor)      │
└────────────────────┘    └──────┬───────────┘
                                 │
                    ┌────────────┴────────────┐
                    │                         │
                    ▼                         ▼
           ┌────────────────┐      ┌──────────────────┐
           │ Job Controller │      │  I/O Redirector  │
           │ (Process Mgmt) │      │  (Pipes & Files) │
           └────────────────┘      └──────────────────┘
```

### Key Components

1. **Input Handler** ([input.c](shell/src/input.c))
   - Read user input from stdin
   - Handle Ctrl-D (EOF) gracefully
   - Buffer management and line editing

2. **Parser** ([parser.c](shell/src/parser.c))
   - Tokenize input into commands and arguments
   - Parse pipelines and redirections
   - Build command execution trees

3. **Executor** ([executor.c](shell/src/executor.c))
   - Fork processes for external commands
   - Set up pipelines between processes
   - Handle I/O redirection
   - Manage process groups

4. **Intrinsics** ([intrinsics.c](shell/src/intrinsics.c))
   - Implement built-in commands
   - Maintain command history
   - Directory navigation logic

5. **Job Controller** ([jobs.c](shell/src/jobs.c))
   - Track background and stopped jobs
   - Handle SIGCHLD for process reaping
   - Manage job IDs and states
   - Foreground/background transitions

6. **Prompt Handler** ([prompt.c](shell/src/prompt.c))
   - Display dynamic shell prompt
   - Show current directory relative to home
   - Display system information

## Prerequisites

### Required
- **Operating System**: Linux (Ubuntu 20.04+, Fedora, Arch, etc.) or macOS
- **Compiler**: GCC 7.0+ or Clang 10.0+ with C99 support
- **Make**: GNU Make 4.0+
- **POSIX Libraries**: Standard C library with POSIX extensions

### Development Tools (Optional)
- **GDB**: For debugging
- **Valgrind**: For memory leak detection
- **Strace**: For system call tracing

## Installation

### Quick Start

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/POSIX-Compliant-Unix-Shell.git
   cd POSIX-Compliant-Unix-Shell
   ```

2. **Navigate to the shell directory**
   ```bash
   cd shell
   ```

3. **Build the shell**
   ```bash
   make
   ```

4. **Run the shell**
   ```bash
   ./shell.out
   ```

### Build Options

**Clean build:**
```bash
make clean
make
```

**Debug build (with symbols):**
```bash
make CFLAGS="-g -O0 -Wall -Wextra"
```

**Release build (optimized):**
```bash
make CFLAGS="-O3 -DNDEBUG"
```

## Usage

### Starting the Shell

```bash
./shell.out
```

The shell will display a prompt in the format:
```
<username@hostname:current_directory>
```

### Basic Command Execution

```bash
# Simple command
ls -la

# Command with arguments
grep "pattern" file.txt

# Multiple commands (sequential)
cd /tmp ; ls ; pwd
```

### Pipelines

```bash
# Two-stage pipeline
ls -l | grep ".txt"

# Multi-stage pipeline
cat file.txt | grep "error" | sort | uniq -c

# Pipeline with redirection
cat input.txt | tr 'a-z' 'A-Z' > output.txt
```

### I/O Redirection

```bash
# Output redirection (overwrite)
echo "Hello" > file.txt

# Output redirection (append)
echo "World" >> file.txt

# Input redirection
sort < unsorted.txt

# Combined redirection
sort < input.txt > sorted.txt

# Error redirection (if implemented)
command 2> errors.log
```

### Background Execution

```bash
# Run in background
sleep 100 &

# Multiple background jobs
./long_process &
./another_process &

# Check running jobs
activities
```

### Job Control

```bash
# List all jobs
activities

# Bring job to foreground
fg 1

# Send job to background
bg 2

# Send signal to process
ping 1234 9    # Send SIGKILL to PID 1234
```

## Built-in Commands

### hop - Directory Navigation

Change the current working directory with enhanced features.

**Syntax:**
```bash
hop [directories...]
```

**Examples:**
```bash
hop                     # Go to home directory
hop ~                   # Go to home directory
hop /usr/local/bin      # Go to absolute path
hop ../..               # Go up two directories
hop -                   # Go to previous directory
hop dir1 dir2 dir3      # Chain multiple hops
```

**Features:**
- Maintains previous directory for `hop -`
- Supports `~` for home directory
- Handles relative and absolute paths
- Multi-argument support for sequential navigation

### reveal - Directory Listing

List directory contents with advanced filtering.

**Syntax:**
```bash
reveal [flags] [path]
```

**Flags:**
- `-a`: Show hidden files (starting with `.`)
- `-l`: Line-by-line output (one entry per line)

**Examples:**
```bash
reveal                  # List current directory
reveal -a               # List all files including hidden
reveal -l               # List in long format (one per line)
reveal -al /home        # List all files in /home, one per line
reveal ~                # List home directory
reveal -                # List previous directory
```

**Features:**
- Alphabetically sorted output
- Support for special paths (`~`, `-`, `.`, `..`)
- Color-coded output (if terminal supports it)
- Handles empty directories gracefully

### log - Command History

Manage and execute commands from history.

**Syntax:**
```bash
log                     # Display all history
log purge               # Clear history
log execute <index>     # Execute command at index
```

**Examples:**
```bash
log                     # Show last 15 commands
log execute 3           # Execute 3rd most recent command
log purge               # Clear all history
```

**Features:**
- Persistent history (saved to `~/.shell_log`)
- Maximum 15 entries (circular buffer)
- 1-based indexing (1 = most recent)
- Automatic duplicate removal

### activities - Job List

Display all running and stopped background jobs.

**Syntax:**
```bash
activities
```

**Output Format:**
```
[PID] : command - State
```

**Example Output:**
```
[1234] : sleep 100 - Running
[1235] : vim file.txt - Stopped
[1236] : ./server & - Running
```

**Features:**
- Sorted alphabetically by command
- Shows PID and state (Running/Stopped)
- Auto-updates when jobs complete

### ping - Signal Sending

Send arbitrary signals to processes.

**Syntax:**
```bash
ping <pid> <signal_number>
```

**Examples:**
```bash
ping 1234 9             # Send SIGKILL (terminate)
ping 1234 15            # Send SIGTERM (polite terminate)
ping 1234 19            # Send SIGSTOP (pause)
ping 1234 18            # Send SIGCONT (resume)
```

**Features:**
- Validates process existence
- Modulo 32 for signal number (prevents invalid signals)
- Confirmation message on success

### fg - Foreground Job

Bring a background or stopped job to the foreground.

**Syntax:**
```bash
fg [job_id]
```

**Examples:**
```bash
fg                      # Foreground most recent job
fg 1                    # Foreground job with ID 1
```

**Features:**
- Resumes stopped jobs
- Transfers terminal control
- Waits for job completion

### bg - Background Job

Resume a stopped job in the background.

**Syntax:**
```bash
bg [job_id]
```

**Examples:**
```bash
bg                      # Background most recent stopped job
bg 2                    # Background job with ID 2
```

**Features:**
- Only affects stopped jobs
- Sends SIGCONT to resume
- Job continues asynchronously

## Advanced Features

### Signal Handling

The shell properly handles Unix signals:

| Signal | Behavior |
|--------|----------|
| `SIGINT` (Ctrl-C) | Terminate foreground job, not the shell |
| `SIGTSTP` (Ctrl-Z) | Stop foreground job |
| `SIGCHLD` | Reap completed background jobs |
| `SIGTTIN/SIGTTOU` | Ignored to prevent shell suspension |

### Process Groups

Each job runs in its own process group (PGID) for proper job control:
- Shell is the session leader
- Each pipeline forms one process group
- Terminal control transferred to foreground job
- Background jobs run without terminal access

### Error Handling

Comprehensive error reporting:
- Command not found
- Permission denied
- Invalid syntax
- File not found
- Process not found
- Signal delivery failures

### Command History Integration

**Execute from history within pipelines:**
```bash
log execute 5 | grep "pattern"
```

This feature allows combining historical commands with new operations.

## Project Structure

```
POSIX-Compliant-Unix-Shell/
├── shell/
│   ├── include/
│   │   ├── shell.h          # Main header with globals and includes
│   │   ├── executor.h       # Command execution interface
│   │   ├── input.h          # Input handling interface
│   │   ├── intrinsics.h     # Built-in commands interface
│   │   ├── jobs.h           # Job control interface
│   │   ├── parser.h         # Command parsing interface
│   │   └── prompt.h         # Prompt display interface
│   │
│   ├── src/
│   │   ├── main.c           # Entry point and signal setup
│   │   ├── executor.c       # External command execution
│   │   ├── input.c          # User input reading
│   │   ├── intrinsics.c     # Built-in command implementations
│   │   ├── jobs.c           # Job control and management
│   │   ├── parser.c         # Command parsing and tokenization
│   │   └── prompt.c         # Prompt generation
│   │
│   ├── Makefile             # Build configuration
│   └── shell.out            # Compiled executable (generated)
│
└── README.md                # This file
```

### File Descriptions

**Headers:**
- **shell.h**: Core definitions, includes, and global variables
- **executor.h**: Interface for command execution engine
- **input.h**: User input handling declarations
- **intrinsics.h**: Built-in command function prototypes
- **jobs.h**: Job control structures and functions
- **parser.h**: Parsing utilities and data structures
- **prompt.h**: Prompt display functions

**Source Files:**
- **main.c**: Program entry, initialization, REPL loop, signal handlers
- **executor.c**: Fork/exec logic, pipeline creation, I/O redirection
- **input.c**: Line reading, buffering, EOF handling
- **intrinsics.c**: Implementation of all built-in commands
- **jobs.c**: Job tracking, background process management, reaping
- **parser.c**: Tokenization, syntax analysis, command tree building
- **prompt.c**: Dynamic prompt generation with path simplification

## Technical Implementation

### Process Creation and Management

**Fork-Exec Pattern:**
```c
pid_t pid = fork();
if (pid == 0) {
    // Child process
    setpgid(0, 0);                    // Create new process group
    execvp(args[0], args);            // Execute command
    exit(EXIT_FAILURE);
} else {
    // Parent process
    setpgid(pid, pgid);               // Add to process group
    if (!background) {
        tcsetpgrp(STDIN_FILENO, pgid); // Give terminal control
        waitpid(pid, &status, WUNTRACED);
        tcsetpgrp(STDIN_FILENO, shell_pgid);
    }
}
```

### Pipeline Implementation

**Multi-process Pipeline:**
```c
int pipes[n-1][2];
for (int i = 0; i < n; i++) {
    if (i < n-1) pipe(pipes[i]);
    
    if (fork() == 0) {
        // Setup input from previous pipe
        if (i > 0) {
            dup2(pipes[i-1][0], STDIN_FILENO);
        }
        // Setup output to next pipe
        if (i < n-1) {
            dup2(pipes[i][1], STDOUT_FILENO);
        }
        // Close all pipe descriptors
        for (int j = 0; j < n-1; j++) {
            close(pipes[j][0]);
            close(pipes[j][1]);
        }
        execvp(cmd[i].argv[0], cmd[i].argv);
        exit(EXIT_FAILURE);
    }
}
```

### I/O Redirection

**File Descriptor Manipulation:**
```c
// Input redirection: cmd < file
int fd = open(filename, O_RDONLY);
dup2(fd, STDIN_FILENO);
close(fd);

// Output redirection: cmd > file
int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
dup2(fd, STDOUT_FILENO);
close(fd);

// Append redirection: cmd >> file
int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
dup2(fd, STDOUT_FILENO);
close(fd);
```

### Job Control Architecture

**Job States:**
```c
typedef enum {
    RUNNING,    // Executing in background
    STOPPED,    // Suspended (Ctrl-Z)
} JobState;

typedef struct {
    pid_t pgid;              // Process group ID
    int job_id;              // User-visible job number
    JobState state;          // Current state
    char command[1024];      // Command string
    bool active;             // Entry is valid
} Job;
```

**Reaping Background Jobs:**
```c
void jobs_reap(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        Job* job = find_job_by_pgid(getpgid(pid));
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            printf("[%d]+ Done\t\t%s\n", job->job_id, job->command);
            job->active = false;
        } else if (WIFSTOPPED(status)) {
            job->state = STOPPED;
        }
    }
}
```

### Command History Implementation

**Circular Buffer:**
```c
#define MAX_LOG_SIZE 15
static char* command_log[MAX_LOG_SIZE];
static int log_count = 0;
static int log_start = 0;

void log_add(const char *command) {
    if (log_count < MAX_LOG_SIZE) {
        command_log[log_count++] = strdup(command);
    } else {
        free(command_log[log_start]);
        command_log[log_start] = strdup(command);
        log_start = (log_start + 1) % MAX_LOG_SIZE;
    }
}
```

### Memory Management

- **Dynamic Allocation**: All command strings and job structures use heap allocation
- **Proper Cleanup**: Free allocated memory before exit
- **No Memory Leaks**: Valgrind-tested for leak-free operation
- **Buffer Safety**: All string operations use safe functions (strncpy, snprintf)

## Examples

### Complex Pipeline Example

```bash
# Find all C files, count lines, sort by count
find . -name "*.c" -exec wc -l {} + | sort -n | tail -10
```

### Background Job Management

```bash
# Start multiple background jobs
sleep 100 &
./server 8080 &
./worker &

# List jobs
activities

# Bring specific job to foreground
fg 2

# (Press Ctrl-Z to stop it)

# Resume in background
bg 2
```

### Directory Navigation Workflow

```bash
hop /usr/local/bin
hop ~/projects/shell
hop -                    # Back to /usr/local/bin
hop -                    # Back to ~/projects/shell
```

### I/O Redirection Combinations

```bash
# Read from file, process, write to file
sort < input.txt | uniq > output.txt

# Append to file while viewing
ls -R / | tee -a full_list.txt

# Complex redirection
(echo "Header"; cat data.txt) > combined.txt
```

### Historical Command Execution

```bash
# Show history
log

# Execute previous grep command
log execute 3

# Chain with new pipeline
log execute 5 | wc -l
```

## Testing

### Manual Testing

**Basic functionality:**
```bash
# Test simple commands
ls
pwd
echo "hello world"

# Test pipelines
ls | grep ".c"
cat file.txt | sort | uniq

# Test redirection
echo "test" > file.txt
cat < file.txt
echo "more" >> file.txt

# Test background jobs
sleep 10 &
activities
```

### Stress Testing

**Multiple pipelines:**
```bash
cat /dev/urandom | head -c 1000000 | md5sum
```

**Many background jobs:**
```bash
for i in {1..10}; do sleep 100 & done
activities
```

**Deep directory navigation:**
```bash
hop /usr/local/bin
hop /var/log
hop ~/
hop -
```

### Error Handling Tests

```bash
# Command not found
nonexistent_command

# Invalid syntax
ls | | grep

# Permission denied
cat /etc/shadow

# Invalid job ID
fg 999
```

### Memory Leak Detection

```bash
# Run under Valgrind
valgrind --leak-check=full --show-leak-kinds=all ./shell.out

# Perform operations and exit cleanly
# Check for "no leaks are possible"
```

## Known Limitations

### Current Implementation

- **No Globbing**: Wildcards (`*`, `?`, `[]`) are not expanded by the shell
  - Workaround: External commands handle their own globbing
  
- **No Environment Variables**: `export`, `unset`, `$VAR` not supported
  - Inherits parent environment
  
- **No Quoting**: Quotes don't prevent word splitting
  - Arguments with spaces must be escaped
  
- **No Command Substitution**: Backticks and `$()` not supported
  
- **No Conditional Execution**: `&&` and `||` operators not implemented
  
- **Limited Error Redirection**: `2>` and `2>&1` not fully supported

### POSIX Compliance

This shell implements core POSIX features but omits:
- Shell scripting (if/then/else, loops)
- Functions and aliases
- Advanced parameter expansion
- Here documents (`<<`)
- Job control with `%` syntax

### Performance Considerations

- History limited to 15 commands
- Maximum 64 concurrent background jobs
- Command line length limited to 1024 characters
- Maximum 64 arguments per command

## Contributing

Contributions are welcome! Please follow these guidelines:

### Code Style

- **Indentation**: 4 spaces (no tabs)
- **Naming**: snake_case for functions and variables
- **Braces**: K&R style (opening brace on same line)
- **Comments**: Explain complex logic and algorithms
- **Headers**: Include guards in all header files

### Development Workflow

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/enhancement`)
3. Write clean, documented code
4. Test thoroughly (including edge cases)
5. Run memory leak detection
6. Commit with clear messages
7. Push to your fork
8. Submit a pull request

### Testing Requirements

- Test with various input combinations
- Verify signal handling behavior
- Check for memory leaks with Valgrind
- Test both interactive and non-interactive modes
- Verify job control with multiple background jobs

### Documentation

- Update README for new features
- Add inline comments for complex code
- Document new built-in commands
- Include usage examples

## License

This project is available for educational and professional review purposes. Please contact the repository owner for licensing information.

## Acknowledgments

### References

- **Advanced Programming in the UNIX Environment** by W. Richard Stevens
- **The Linux Programming Interface** by Michael Kerrisk
- **POSIX.1-2017 Standard** (IEEE Std 1003.1-2017)
- GNU Bash source code for implementation patterns

### Educational Resources

- [Unix System Calls Tutorial](https://www.cs.columbia.edu/~jae/4118/L06-process.html)
- [Process Control](https://www.gnu.org/software/libc/manual/html_node/Processes.html)
- [Job Control](https://www.gnu.org/software/libc/manual/html_node/Job-Control.html)

## Troubleshooting

### Common Issues

**Shell doesn't respond to Ctrl-C:**
- Verify signal handlers are installed
- Check process group settings
- Ensure foreground job has terminal control

**Background jobs become zombies:**
- Verify `jobs_reap()` is called in main loop
- Check `waitpid` with `WNOHANG` flag
- Ensure SIGCHLD is not blocked

**Pipeline doesn't work:**
- Verify all pipe file descriptors are closed in children
- Check dup2 is called before exec
- Ensure parent closes pipe ends

**Job control issues:**
- Run `stty -a` to check terminal settings
- Verify shell is session leader
- Check `tcsetpgrp` calls

## Future Enhancements

- [ ] Tab completion for commands and paths
- [ ] Command-line editing (Readline support)
- [ ] Environment variable support
- [ ] Wildcard expansion (globbing)
- [ ] Conditional execution (`&&`, `||`)
- [ ] Shell scripting support
- [ ] Configuration file (~/.shellrc)
- [ ] Color customization
- [ ] Alias support
- [ ] History search (Ctrl-R)
- [ ] Command suggestions
- [ ] Syntax highlighting

---

**Built with C99 and POSIX standards**