### Job Control Features

## how job control works

Job control is the ability to selectively stop (suspend) the execution of processes and continue (resume) their execution at a later point. It allows you to switch between foreground - fg - and background - bg - tasks.



* **Foreground Job - FG:** The process currently attached to the terminal
* **Background Job- BG:** A process running asynchronously. doesn't read from the terminal, allowing you to use the shell prompt immediately.
* **Process Group:** A collection of related processes (e.g., a pipeline `ls | grep`). Signals sent to the group affect all processes in that pipeline.

---
# Add example, ... sleep 1000 & , fg 1, ctrl+Z , bg 1 ... multiple ... screenshots of jobs list ... 

## 2. Command & Syntax Reference

The following commands and operators are now supported in our `s3`:

| Command / Syntax | Description | Example |
| :--- | :--- | :--- |
| **`&`**| **Background Execution.** Placed at the end of a command, this instructs the shell to execute the command in a new process group in the background. The prompt returns immediately. | `sleep 20 &` |
| **`Ctrl + Z`** | **Suspend Job.** Sends the `SIGTSTP` signal to the current foreground process group. The job is paused and moved to the background, and the shell prompt returns. | *(Press while running)* |
| **`Ctrl + C`** | **Interrupt Job.** Sends the `SIGINT` signal to the current foreground process group, terminating it. | *(Press while running)* |
| **`jobs`** | **List Jobs/ Job Table.** Displays the status of all active jobs tracked by our shell (Running or Stopped) along with their Job ID. | `jobs` |
| **`fg <job_id>`** | **Foreground.** Brings a background or stopped job to the foreground. The shell waits for this job to finish (or stop again) before returning the prompt. | `fg 1` |
| **`bg <job_id>`** | **Background.** Resumes a *stopped* job execution in the background. Useful if you started a command without `&` but want to free up our terminal. | `bg 1` |
| **`exit`** | **exits shell** `Ctrl+C` - has different functionality now | `exit` |

---

## 3. Implementation Details: What we changed

To implement this functionality, specific changes were made to our shell architecture to manage process groups and signals correctly.

| Component                                     | Change                                                                  | Why                                                                                                                                            |
| --------------------------------------------- | ----------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| **A. Lexer (`lexer_fsm.c`)**                  | Added `TOKEN_AMPERSAND` and FSM logic for `&`                           | Allows the parser to detect background commands and set the `is_bg` flag instead of treating `&` as a normal character.                        |
| **B. Process Groups (`s3.c`)**                | Added `setpgid(0,0)` in children and `setpgid(pid,pid)` in parent       | Ensures each launched job gets its own process group so terminal signals (`Ctrl+C`, `Ctrl+Z`) target the job, not the shell.                   |
| **C. Terminal Control (`s3.c`)**              | Added `tcsetpgrp(STDIN_FILENO, pgid)` and restored shell PGID afterward | Transfers terminal ownership to foreground jobs, enabling them to receive input and signals; returns control to the shell when they stop/exit. |
| **D. Signal Ignoring (`s3main.c`)**           | Shell now ignores `SIGINT`, `SIGTSTP`, etc. with `signal(..., SIG_IGN)` | Prevents the shell from being interrupted or stopped; signals still reach the foreground job because of proper PGIDs.                          |
| **E. Smart Waiting (`jobs.c`)**               | Replaced simple `waitpid` with `wait_for_job` using `WUNTRACED`         | Detects not only terminated jobs but also stopped ones (from `Ctrl+Z`), updating the job table and printing status messages.                   |
| **F. Built-in Commands (`fg`, `bg`, `jobs`)** | Added internal handlers in `process_input` for these keywords           | These commands don’t exist as executables; they operate on the shell’s job table and use `SIGCONT` to resume stopped jobs.                  |
