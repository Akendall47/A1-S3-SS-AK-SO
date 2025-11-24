# Lexer Integration

Due to the parameters of the brief, we started the project with simple string manipulating fucntions to help us parse the command line and produce quick solutions. 
Overtime, more of these fucntions built-up and created a more messy solution, some fucntions making others possibly redudant upon merging, so naturally a more elegant solution became obvious, particualry as we looked to add our own enhacements to the dafult brief, especially our previous way to tackle the `echo` commmand became complex.
A Lexer implementated by a state machine, in which our main s3 functions could call from and tokenize the command line accordingly, inheriting design from functions we had previously written. 
This also had efficiency improvments, as a lexer would become 1 parse, where as reading, inserting or rewriting the command line would be seen as far slower. Perhaps O(n) vs O(n^2)
-------------------------------

## What Changed and Why

### removed Code (Problematic Parts)

| Function |-------  | Why Removed |
|----------|-----|-------------|
| `QuoteState` struct | -- | Replaced by FSM state tracking |
| `update_quote_state()` |----| FSM handles state transitions automatically |
| `is_quoted()` | -----| FSM knows current state internally |
| `find_unquoted_char()` | ----| FSM processes all characters once |
| `insert_spaces_around_ops()` | ---- | No preprocessing needed with FSM |
| `strip_outer_quotes()` | ---| FSM strips quotes during tokenization |



| Function | ---- | What Changed |
|----------|-------------|--------------|
| `parse_command()` |-----| Uses FSM for tokenization, keeps your args extraction logic |
| `find_redirection_operator()` | ---- | Uses tokens instead of string scanning, keeps your detection logic |
| `command_has_pipes()` | ----| Uses tokens to find pipes, keeps your concept |
| `split_pipeline()` | ----| Uses tokens to split, keeps your stage reconstruction |
| `read_command_line()` | ----| Removed preprocessing, kept everything else |


### The Problem 

Previosuly we would preprocess the input, tracks " " quotes manually in multiple places, handle edge cases poorly, process string multiple times 

### FSM Solution:

I have notes on My ipad thats good for this
i will draw up diagram nicer 
then append parts of this seciton to final readme
