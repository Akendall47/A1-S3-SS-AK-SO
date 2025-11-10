Flush STD I/O ? first : fflush(0)

open() calls error checked? 

currently only consider single redireciton operator

Use the open system call to access the input or output file before redirecting streams with dup2. 

 specify flags such as O_RDONLY, O_WRONLY, or O_CREAT as needed.
Note that the > operator creates the output file if it doesn't exist, 
or overwrites it if it does. The >> operator appends to the file, and also creates it if it doesn't exist.