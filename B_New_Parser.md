parser needed to be updated to handle special charcters better
especially to handle echo 


therefore also spotted that connected cmds also not tokenized/handled correctly:

echo [/home/akendall/Documents/A1-S3-SS-AK-SO s3]$ echo hi>>file
would print and not redirect eg:
hi>>file


gcc *.c -o s3

2> and 2>> (stderr redirection)
>& (duplication)
background jobs &
variable expansion $HOME
command substitution $(...)
tab completion
signal handling (Ctrl-C, Ctrl-Z)
job control (fg, bg, jobs)

### FROM HIS GITHUB
Further enhaancmenets;
One such feature is globbing, which allows wildcard-based pattern matching for filenames using characters like * and ?.

You can also explore foreground and background job control, allowing commands to run concurrently (although we will be studying concurrency and its related issues later in the module), along with the ability to bring jobs to the foreground or send them to the background using fg and bg.

Another enhancement is supporting both input and output redirection in the same command line. For example:

$ sort < unsorted.txt > sorted.txt
Bash
You may consider adding support for alternative syntax for tee-like behavior, or expanding piping capabilities to allow for more complex routing of data streams.

On the user interface side, features such as command history, tab completion, and custom prompts can greatly improve usability.
