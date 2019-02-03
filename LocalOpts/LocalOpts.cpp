// 5984 S18 Assignment 1: LocalOpts.cpp
// PID: usman
////////////////////////////////////////////////////////////////////////////////

#include "unordered_map"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"

#include <iostream>

using namespace llvm;

namespace {
  class LocalOpts : public BasicBlockPass {
  public:
    static char ID;

    LocalOpts() : BasicBlockPass(ID) { }
    ~LocalOpts() { }

    // We don't modify the program, so we preserve all analyses
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }

    bool doInitialization(Module &M) override {

      return false;
    }

    bool doFinalization(Module &M) {

      return false;
    }

    bool runOnBasicBlock(BasicBlock &BB) override {
      bool wasModified = false;

      return wasModified;
    }
    
  };
}

// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char LocalOpts::ID = 0;
static RegisterPass<LocalOpts> X("local-opts", "5984: Local Optimizations", false, false);
