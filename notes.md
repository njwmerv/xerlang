26/05/2026:
- Did 2/3 of before, just "need" tests (later...)
- Learn LLVM IR !
- Learn x86 ASM !
- Made sketch of what to do (in `main.cpp`):
  - Conversion to IR
  - Optimizations (what exactly idk so far...)
  - Code Generation

19/05/2026:
- Realized, as of right now, variables are scoped to entire IF trees, and not just their clause.
    - Possibly make `scope` a `vector<ASTNode*>`?
    - Stack of Hash-maps, try that...
- More rigorous testing needed probably...
- Need to calculate frame offsets for variables... also feels like there's gotta be a better way than 