### consider `forkpty( master_fd, slave name, settings, window size)`
creates a pseudo terminal
so can test like talking to a real user 
then we can RD and WR to a test file 
#### forkpty actions
creates anew PTY pair -> forks process
auto connects: childs stdin -> PTY Slave
returns the PTY master fd to the parent 

#### Run the Test
```bash
gcc test_shell.c -o test_shell 
gcc s3.c s3main.c -o s3
./test_shell
```
### or self control 
```bash
gcc s3.c s3main.c -o s3
./s3test
```

### consider multiple layers?
assertion?  `unity` - probably unneccsary as the hard part if the pseudo terminal... not unity 
just a nice extra 

interation? `forkpty` in .c
