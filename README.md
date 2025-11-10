| Section                      | Description                                                 | Assigned To                |     Suggested Branch       | Status          |           |
| ---------------------------- | ----------------------------------------------------------- | -------------------------  | -------------------------- | --------------  | --------- |
| 1. Basic Commands            | Implement fork/execvp, basic command parsing                |                            | `main` (done)              | ✅              |           |
| 2. Commands with Redirection | Support `>`, `<`, `>>` using `dup2`, `open`                 |                            | `feature/redirection`      | ✅              |           |
| 3. Support for `cd`          | Handle `cd`, `cd -`, `cd ..`, update prompt with `getcwd()` |                            | `feature/cd-support`       | ⏳              |           |
| 4. Commands with Pipes       | Implement `                                                 |                            | `feature/pipes`            | ⏳              |           |
| 5. Batched Commands          | Handle `;` separated commands                               |                            | `feature/batch`            | ⏳              |           |
| 6. PE 1: Subshells           | Implement `( ... )` grouped commands as subshells           |                            | `feature/subshells`        | ⏳              |           |
| 7. PE 2: Nested Subshells    | Extend subshell support for nested parentheses              |                            | `feature/nested-subshells` | ⏳              |           |
| 8. Further Enhancements      | Optional: globbing, job control, command history            |                            | `feature/enhancements`     | 💤              |           |
