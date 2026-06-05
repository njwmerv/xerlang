# Xerlang

Inspired by taking a course about compiling a basic C-like language, 
I wanted to make my own language and compiler for it, with its own
basic features on top of what that language provides, such as:
- structs
- more basic types, e.g. char & bool
- global variables
- for-loops
- other operations, e.g. bitwise OPs
- more control statements, e.g. break
- variable declarations _NOT_ at the start of local scope
- early return
- can actually run in a normal OS environment and not some fake one
  - will be making this for Linux

NOTE: Seems like since changing the course to use ARM instead of
MIPS, they've made changes to the language, but it still seems
to be lacking these features.

Example Program: Program that reads each character from `stdin` and prints them one by one.
```
char QUIT = 'q';

main : () -> int {
    char c;
    while (true) {
        c = read();
        if (c == QUIT) {
            break;
        }
        print(c);
        print('\n');
    }
}
```
Example Program: Hello World (string literals and array index to come eventually)
```
main : () -> int {
    # "Hello, World!\n"
    char@ string = new char [14];
    @(string + 0) = 'H';
    @(string + 1) = 'e';
    @(string + 2) = 'l';
    @(string + 3) = 'l';
    @(string + 4) = 'o';
    @(string + 5) = ',';
    @(string + 6) = ' ';
    @(string + 7) = 'W';
    @(string + 8) = 'o';
    @(string + 9) = 'r';
    @(string + 10) = 'l';
    @(string + 11) = 'd';
    @(string + 12) = '!';
    @(string + 13) = '\n';

    for (int i = 0; i < 14; i++) {
        print(@(string + i));
    }
}
```
