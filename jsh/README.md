# jsh

## Author
- Name: **JINLOCK CHOI**
- Student No.: **520370990012**
- E-mail: **jinlock99@sjtu.edu.cn**

## Supported Features
- Built-in Commands:  
    * `exit`
    * `pwd`
    * `cd`
    * `jobs`
- Read and Parse Arguments
- Single Commands
- Single Commands with Arguments
- Output Redirection
- Input Redirection
- Bash Style Redirection Syntax
- Pipes
- CTRL-D
- CTRL-C
- Quotes
- Errors Handling
- &: Background Running

# Change Log
## Milestone 1
### 09/28/2024
1. Write a working read-parse-execute loop and an exit command
2. Handle single commands without arguments (e.g. ls)
3. Support commands with arguments (e.g. apt-get update or pkgin update)
### 09/29/2024
4. File I/O redirection:
    - 4.1. Output redirection by overwriting a file (e.g. echo 123 > 1.txt);
    - 4.2. Output redirection by appending to a file (e.g. echo 465 >> 1.txt);
    - 4.3. Input redirection (e.g. cat < 1.txt);
    - 4.4. Combine 4.1 and 4.2 with 4.3;
5. Support for bash style redirection syntax (e.g. cat < 1.txt 2.txt > 3.txt 4.txt); 

## Milestone 2
### 09/30/2024
6. Pipes:
    - 6.1. Basic pipe support (e.g. echo 123 | grep 1);
    - 6.2. Run all 'stages' of piped process in parallel. (e.g. yes ece482 | grep 482);
    - 6.3. Extend 6.2 to support requirements 4. and 5. (e.g. cat < 1.txt 2.txt | grep 1 > 3.txt);
    - 6.4. Extend 6.3 to support arbitrarily deep "cascade pipes" (e.g. echo 123 | grep 1 | grep 1 | grep 1)
7. Support CTRL-D (similar to bash when there is no/an unfinished command);
### 10/05/2024
8. Internal commands:
    - 8.1. Implement pwd as a built-in command;
    - 8.2. Allow changing working directory using cd;
    - 8.3. Allow pwd to be piped or redirected as specified in requirement 4.;
### 10/08/2024
9. Support CTRL-C:
    - 9.1. Properly handle CTRL-C in the case of requirement 4.;
    - 9.2. Extend 9.1 to support subtasks 6.1 to 6.3;
    - 9.3. Extend 9.2 to support requirement 7., especially on an incomplete input;
    - 9.4. Extend 9.3 to support requirement 6.;

## Milestone 3
### 2024/10/11:
- Branch: feat/tokenizer 
    - Separate parser and tokenizer
    - Merge into p1m3
- Branch: feat/quotes
    - Support quotes:
        - 10.1. Handle single and double quotes (e.gm. echo "de'f' ghi" '123"a"bc' a b c);
        - 10.2. Extend 10.1 to support requirement 4. and subtasks 6.1 to 6.3;
        - 10.3. Extend 10.2 in the case of incomplete quotes (e.g. Input echo "de, hit enter and input cd");
        - 10.4. Extend 10.3 to support requirements 4. and 6., together with subtask 9.3;
    - Wait for the command to be completed when encountering >, <, or |: [3+3]
        - 11.1. Support requirements 3. and 4. together with subtasks 6.1 to 6.3;
        - 11.2. Extend 11.1 to support requirement 9.; (Previous case output the two lines [1] done /bin/ls & and [2] running /bin/ls | cat &);
### 2024/10/12
- Branch: feat/quotes
    - Error detected: 
        - p1m3 - case 3:  `"echo" "123" "|" | cat > 1.txt`
            - Branch: feat/quotes-debug:
                - Add checking not_string logic in tokenizer()
                - FIX!
- Branch: feat/error
    - Handle syntax errors
    - Handle io errors
    - Handle execution errors
- Handle errors for all supported features. [10]  
    - Note: a list of test cases will be published at a later stage. Marks will be awarded based on the number of casesthat are correctly handled, i.e. if only if:
    - Aprecise error message is displayed (e.g. simply saying "error happened!" is not enough);
    - The program continues executing normally after the error is identified and handled;
### 2024/10/13
- Branch: feat/bg
    - A command ending with an & should be run in background. [10+5]
        - 13.1. For any background job, print out the command line, prepended with the job ID (e.g. if the two lines /bin/ls & and /bin/ls | cat & are input the output could be the two lines [1] /bin/ls & and [2] /bin/ls | cat & );
        - 13.2. Implement the command jobs which prints a list of background tasks together with their running states (e.g. in the previous case output the two lines [1] done /bin/ls & and [2] running /bin/ls | cat &);
- Branch: feat/improve
    - Make codes neat according to clang-tidy 
### 2024/10/16
- Branch: feat/improve
    - Error fix:
        - Remove unneccesary header <sys/types.h>
        - Add <sys/stat.h> to utilize constants for open()
        