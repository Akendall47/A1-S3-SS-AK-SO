#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

PASS=0
FAIL=0
TOTAL=0

run_test(){
    local test_name="$1"
    local commands="$2"
    local expected="$3"

    ((TOTAL++))
    echo -n "Test $TOTAL: $test_name... "

    # Run commands in shell
    result=$(echo "$commands" | timeout 3 ./s3 2>&1)

    # Check if expected string is in result
    if echo "$result" | grep -q "$expected"; then
        echo -e "${GREEN} PASS${NC}"
        ((PASS++))
        return 0
    else
        echo -e "${RED} FAIL${NC}"
        echo "  Expected to find: '$expected'"
        echo "  Got: $result" | head -3
        ((FAIL++))
        return 1
    fi
}


echo "    S3 Shell Test Suite"
echo ""

# Compile the shell first
echo -e "${BLUE}Compiling s3 shell...${NC}"
gcc -Wall -Wextra -g s3main.c s3.c jobs.c lexer_fsm.c terminal_control.c -o s3
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed!${NC}"
    exit 1
fi
echo -e "${GREEN}Compilation successful!${NC}"
echo ""

#basics
echo -e "${YELLOW}[Basic Commands]${NC}"
run_test "echo command" "echo hello" "hello"
run_test "pwd command" "pwd" "A1-S3-SS-AK-SO"
run_test "ls command" "ls" "s3"

echo ""

# IO redirects
echo -e "${YELLOW}[I/O Redirection]${NC}"
run_test "output redirection" "echo test123 > /tmp/s3_test1.txt; cat /tmp/s3_test1.txt" "test123"
run_test "append redirection" "echo line1 > /tmp/s3_test2.txt; echo line2 >> /tmp/s3_test2.txt; cat /tmp/s3_test2.txt" "line2"
echo "input_data" > /tmp/s3_test3.txt
run_test "input redirection" "cat < /tmp/s3_test3.txt" "input_data"

echo ""

# pipelines
echo -e "${YELLOW}[Pipelines]${NC}"
run_test "simple pipe" "echo pipe_test | cat" "pipe_test"
run_test "multi-stage pipe" "echo 'one two three' | wc -w" "3"
run_test "grep pipe" "echo 'find me' | grep find" "find me"

echo ""

# =buidlt ins
echo -e "${YELLOW}[Built-in Commands]${NC}"
run_test "cd command" "cd /tmp; pwd" "/tmp"
run_test "cd - (previous dir)" "cd /tmp; cd -; pwd" "A1-S3-SS-AK-SO"
run_test "cd ~ (home)" "cd ~; pwd" "$HOME"

echo ""

# multi
echo -e "${YELLOW}[Multiple Commands]${NC}"
run_test "semicolon separator" "echo first; echo second" "second"
run_test "three commands" "echo A; echo B; echo C" "C"

echo ""

# subshells
echo -e "${YELLOW}[Subshells]${NC}"
run_test "subshell execution" "(echo subshell_test)" "subshell_test"
run_test "subshell isolation" "(cd /tmp); pwd" "A1-S3-SS-AK-SO"

echo ""

# erros
echo -e "${YELLOW}[Error Handling]${NC}"
run_test "non-existent command" "nonexistent_cmd_xyz" "No such file"
run_test "missing file" "cat /tmp/nonexistent_file_xyz.txt" "No such file"

echo ""

# quotes / specials
echo -e "${YELLOW}[Quotes & Special Characters]${NC}"
run_test "double quotes" "echo \"hello world\"" "hello world"
run_test "single quotes" "echo 'single quoted'" "single quoted"

echo ""

# globbing
echo -e "${YELLOW}[Globbing]${NC}"
run_test "glob *.c files" "ls *.c | head -1" ".c"
run_test "glob *.h files" "ls *.h | head -1" ".h"
run_test "no glob expansion" "echo test" "test"

echo ""

# cleanup
rm -f /tmp/s3_test1.txt /tmp/s3_test2.txt /tmp/s3_test3.txt

# Summary
echo -e "${BLUE}========================================${NC}"
if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}All tests passed! ($PASS/$TOTAL)${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed: ${GREEN}$PASS passed${NC}, ${RED}$FAIL failed${NC} (Total: $TOTAL)"
    exit 1
fi
