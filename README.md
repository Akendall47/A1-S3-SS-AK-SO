| Section                      | Description                                                 | Assigned To                |    Suggested Branch| Status          |           |
| ---------------------------- | ----------------------------------------------------------- | -------------------------  | -----------------  | -------------   | --------- |
| 1. Basic Commands            | Implement fork/execvp, basic command parsing                |                            | `main` (done)      | ✅              |           |
| 2. Commands with Redirection | Support `>`, `<`, `>>` using `dup2`, `open`                 |                            | `redirection`      | ✅              |           |
| 3. Support for `cd`          | Handle `cd`, `cd -`, `cd ..`, update prompt with `getcwd()` |                            | `cd-support`       | ✅              |           |
| 4. Commands with Pipes       | Implement `                                                 |                            | `pipes`            | ✅              |           |
| 5. Batched Commands          | Handle `;` separated commands                               |                            | `batch`            | ✅              |           |
| 6. PE 1: Subshells           | Implement `( ... )` grouped commands as subshells           |                            | `subshells`        | ✅              |           |
| 7. PE 2: Nested Subshells    | Extend subshell support for nested parentheses              |                            | `nested-subshells` | ✅              |           |
| 8. Custom Lexer FSM          | Optional: globbing, job control, command history            |                            | `enhancements`     | ...              |           |
| 9. job Control               | Optional: globbing, job control, command history            |                            | `enhancements`     | ...              |           |

## Testing Progress
A - SETUP  <br>
B - Core commands <br>
C - Subshell Logic <br>
D -  Enhanced Logic <br>

| Test Phase                  | Description                                                    | Notes                               | Status |
| --------------------------- | -------------------------------------------------------------- | ----------------------------------- | ------ |
| **A. PTY Test Harness         | Build `forkpty` based harness for automated shell interaction  | Core foundation for tests         | initial|
| *B. Basic Commands Tests     | Tests for `fork/execvp`, parsing, and simple commands          | Includes `echo`, `ls`, `exit`      | ⏳     |
| *B. Redirection Tests        | Validate `<`, `>`, `>>` and error handling                     | Append vs overwrite behavior       | ⏳     |
| *B. `cd` Tests               | Test `cd`, `cd -`, `cd ..`, prompt updates                     | Includes error cases               | ⏳     |
| *B. Pipe Tests               | Single and multi-stage pipelines                               | Chained commands                   | ⏳     |
| *B. Batch Command Tests      | Commands separated with `;`                                    | Sequential execution               | ⏳     |
| *C. Subshell Tests           | `( … )` subshell evaluation                                    | Isolated env behavior              | ⏳     |
| *C. Nested Subshell Tests    | Multiple layers of `( ( … ) )`                                 | Deep nesting                       | ⏳     |
| *D. Enhancement Tests (Slot) | Future features: globbing, job control, history, etc.          | TBD                                | 💤     |
| *D. Extra Test Slot A        | Reserved for additional enhancement tests                      | TBD                                | 💤     |
| *D. Extra Test Slot B        | Reserved for additional enhancement tests                      | TBD                                | 💤     |