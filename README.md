# Project: Implementing Basic Shell

This project is to implement basic shell named **jsh**.

## Author

- Name: **Jinlock Choi**
- Email:
    - jinlock99@sjtu.edu.cn
    - teampoclain@gmail.com
- Location:
    - *Seoul, Korea*
    - *Shanghai, China*

## Compiling and Running

**compile**: clang-16 *.c -std=gnu17 -Wvla -Wall -Wextra -Werror -Wpedantic -Wno-unused-result -Wconversion  
**run**: `./jsh`

## Brief documentation

- **Supported Features**
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