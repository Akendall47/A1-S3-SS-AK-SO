# S3 Shell - Software Systems Shell

[GitHub](https://github.com/Akendall47/A1-S3-SS-AK-SO)

**Assignment 1: Shell Implementation**
---

## Table of Contents
1. [Building and Running](#building-and-running)
2. [Basic Commands](#1-basic-commands)
3. [Commands with Redirection](#2-commands-with-redirection)
4. [Support for cd](#3-support-for-cd)
5. [Commands with Pipes](#4-commands-with-pipes)
6. [Batched Commands](#5-batched-commands)
7. [Proposed Extension 1: Subshells](#6-proposed-extension-1-subshells)
8. [Proposed Extension 2: Nested Subshells](#7-proposed-extension-2-nested-subshells)
9. [Further Enhancements](#8-further-enhancements)
10. [Testing](#testing)
11. [Code Quality](#code-quality)

---

## Building and Running

### Compilation
```bash
gcc -Wall -Wextra -g s3main.c s3.c jobs.c lexer_fsm.c terminal_control.c -o s3
```

### Running the Shell
```bash
./s3
```

### Running Tests
```bash
./run_tests.sh
```

---

## 1. Basic Commands

**Status:** ✅ **Fully Implemented**
### Key Functions
- `launch_program()` - Forks and launches basic commands
- `child()` - Executes `execvp()` in child process
- `parse_command()` - Tokenizes command line using custom FSM lexer

## 2. Commands with Redirection

**Status:** ✅ **Fully Implemented**

### Key Functions
- `launch_program_with_redirection()` - Handles commands with redirection
- `child_with_output_redirected()` - Redirects stdout to file
- `child_with_input_redirected()` - Redirects stdin from file
- `find_redirection_in_line()` - Detects redirection operators

### Supported > , >>, <

## 3. Support for cd

**Status:** ✅ **Fully Implemented**

### Key Functions
- `run_cd()` - Changes directory using `chdir()`
- `is_cd()` - Detects cd commands
- `construct_shell_prompt()` - Updates prompt with current directory
---

## 4. Commands with Pipes

**Status:** ✅ **Fully Implemented**

### Key Functions
- `launch_pipeline()` - Manages multi-stage pipelines
- `command_has_pipes()` - Detects pipe operators
- `split_pipeline()` - Tokenizes pipeline stages
- `find_redirection_operator()` - Handles redirection within pipelines

### Features
- Single-stage pipes (`cmd1 | cmd2`)
- Multi-stage pipes (`cmd1 | cmd2 | cmd3 | ...`)


## 5. Batched Commands

**Status:** ✅ **Fully Implemented**

### Key Functions
- `process_input()` - Parses and executes batched commands
- `trim_whitespace()` - Cleans up command tokens

### Features
- Sequential execution of multiple commands
- Independent execution (failure of one doesn't stop others)
- Works with pipes, redirection, and subshells
- Proper parenthesis tracking for subshells
---

## 6. Proposed Extension 1: Subshells

**Status:** ✅ **Fully Implemented**

### Key Functions
- `run_subshell()` - Forks and executes subshell
- `is_subshell()` - Detects subshell syntax

---

## 7. Proposed Extension 2: Nested Subshells

**Status:** ✅ **Fully Implemented**


### Key Functions
- `process_input()` - Tracks parenthesis depth for nested subshells
- `run_subshell()` - Recursively handles nested subshells

### Features
- Unlimited nesting depth
- Proper parenthesis matching
- Recursive execution model
- Isolation at each level

---

## 8. Further Enhancements

### Job Control

#### Features
- Background job execution with `&`
- Job listing with `jobs` command
- Foreground job control with `fg [job_id]`
- Background job continuation with `bg [job_id]`
- Job state tracking (RUNNING, STOPPED)
- Process group management
- Signal handling (SIGINT, SIGTSTP, SIGTTOU, SIGTTIN)

#### Key Functions
- `init_jobs()` - Initializes job table
- `add_job()` - Adds job to tracking table
- `remove_job_by_pgid()` - Removes completed jobs
- `find_job_by_id()` - Locates job by ID
- `print_jobs()` - Lists all jobs
- `wait_for_job()` - Waits for foreground job with WUNTRACED support
- `put_job_in_foreground()` - Brings job to foreground
- `put_job_in_background()` - Resumes job in background

#### Example Commands
```bash
[/home/user s3]$ sleep 30 &
[1] 12345
[/home/user s3]$ jobs
[1] RUNNING    sleep 30 &
[/home/user s3]$ fg 1
# Press Ctrl-Z to suspend
[1] STOPPED    sleep 30
[/home/user s3]$ bg 1
[1] sleep 30 &
[/home/user s3]$ jobs
[1] RUNNING    sleep 30 &
```

### Custom FSM-Based Lexer

#### Features
- Finite State Machine for robust tokenization
- Quote handling (single and double quotes)
- Escape sequence support (`\`)
- Operator recognition (|, <, >, >>, ;, (, ), &)
- Whitespace handling
- Error detection and reporting

### Command History and Terminal Control

#### Features
- Arrow key navigation (up/down for history, left/right for editing)
- Command history with duplicate prevention
- Raw mode terminal control for character-by-character input
- Backspace support with proper cursor positioning
- In-line editing capabilities

#### Key Functions
- `enable_raw_mode()` - Enables raw terminal mode for immediate character input
- `disable_raw_mode()` - Restores canonical terminal mode
- `add_history()` - Adds commands to history linked list
- `move_cursor_left()` / `move_cursor_right()` - Cursor movement control
- `clear_and_redraw()` - Redraws prompt and command line

### Filename Globbing

#### Features
- Wildcard expansion for `*`, `?`, and `[...]` patterns
- Smart detection - only globs when wildcard characters are present
- Multiple argument globbing support
- Seamless integration with all command types

#### Key Function
- `globbing()` - Detects wildcard characters and expands filenames using `glob()`

#### Example Commands
```bash
[/home/user s3]$ ls *.c
jobs.c  lexer_fsm.c  s3.c  s3main.c  terminal_control.c
[/home/user s3]$ echo *.h
jobs.h lexer_fsm.h s3.h terminal_control.h
[/home/user s3]$ cat s3*.c
# Displays contents of all files matching s3*.c
```

## Testing

### Automated Test Suite
with full Test coverage
may need manual etsting for ctrlZ and ctrlz

```bash
# Run full test suite
./run_tests.sh

# Expected output: All tests passed! (23/23)
```
## Code Quality

### Code Organization
- **s3main.c** - Main shell loop and initialization
- **s3.c** - Core shell functionality
- **s3.h** - Function declarations and constants
- **jobs.c** - Job control implementation
- **jobs.h** - Job control interface
- **lexer_fsm.c** - FSM-based lexer implementation
- **lexer_fsm.h** - Lexer interface
- **terminal_control.c** - Functions for terminal control
- **terminal_control.h** - Terminal interface

## Known Limitations and future plans
- Implemented most shell functionality except from tab completition. 
