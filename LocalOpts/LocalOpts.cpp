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
    unsigned identities;
    unsigned folding;
    unsigned reduction;
    LocalOpts() : BasicBlockPass(ID) { }
    ~LocalOpts() { }

    // We don't modify the program, so we preserve all analyses
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }

    bool doInitialization(Module &M) override {
      identities = 0;
      folding = 0;
      reduction = 0;
      return false;
    }

    bool doFinalization(Module &M) {
      outs() << "Transformations applied:\n";
      outs() << "Algebraic identities: " << identities << "\n";
      outs() << "Constant folding: " << folding << "\n";
      outs() << "Strength reduction: " << reduction << "\n";
      return false;
    }

    bool runOnBasicBlock(BasicBlock &BB) override {
      bool wasModified = false;

      for (auto& I : BB) {
        switch (I.getOpcode()) {
          case Instruction::Add:
          case Instruction::FAdd:
          case Instruction::Sub:
          case Instruction::FSub:
          case Instruction::Mul:
          case Instruction::FMul:
          case Instruction::UDiv:
          case Instruction::SDiv:
          case Instruction::FDiv:
          {
            break;
          }
          default: break;
        }  // end switch for opcode
      }  // end I for loop
      
      return wasModified;
    }

  };
}

// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char LocalOpts::ID = 0;
static RegisterPass<LocalOpts> X("local-opts", "5984: Local Optimizations", false, false);
