Local Optimizations in LLVM 6
=============================


LocalOpts.cpp contains three types of optimizations:
1. Algebraic identities
2. Constant folding
3. Strength reduction

Make with the "make" command.
You will have to go into both folders and make seperately.

Output files will be:
  LocalOpts.so (pass name "-local-opts" ) 
  FunctionInfo.so (pass name  "function-info")

