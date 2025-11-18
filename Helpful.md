**File Structure**

`s3main.c` → main entry point (start program)  
`s3.c` → function implementations (shell logic)  
`s3.h` → header file (function declarations & shared definitions)  

**Compile:**
should now be more specific with test script
```bash
gcc s3.c s3main.c -o s3test
