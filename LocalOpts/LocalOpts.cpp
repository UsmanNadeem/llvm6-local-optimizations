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
    // for tracking opts
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

    // final output
    bool doFinalization(Module &M) {
      outs() << "Transformations applied:\n";
      outs() << "Algebraic identities: " << identities << "\n";
      outs() << "Constant folding: " << folding << "\n";
      outs() << "Strength reduction: " << reduction << "\n";
      return false;
    }

    // function to handle identities
    bool doIdentities(BasicBlock &BB) {
      bool wasModifiedRet = false;
      // we are using range based loop and cant remove instructions while 
      // iterating to keep this vector
      std::vector<Instruction*> toRemove;

      for (auto& I : BB) {
        switch (I.getOpcode()) {
          // check if any operand has a 0 value
          case Instruction::Add:
          case Instruction::FAdd:
          {
            if (Constant* constVal = dyn_cast<Constant>(I.getOperand(0))) {
              if (constVal->isZeroValue()) {
                I.replaceAllUsesWith(I.getOperand(1));   
                toRemove.push_back(&I);
              }
            } else if (Constant* constVal = dyn_cast<Constant>(I.getOperand(1))) {
              if (constVal->isZeroValue()) {
                I.replaceAllUsesWith(I.getOperand(0));   
                toRemove.push_back(&I);
              }
            }
            break;
          }
          // check if subtracting 0 value from x
          // or doing x-x
          case Instruction::Sub:
          case Instruction::FSub:
          {
            if (Constant* constVal = dyn_cast<Constant>(I.getOperand(1))) {
              if (constVal->isZeroValue()) {
                I.replaceAllUsesWith(I.getOperand(0));   
                toRemove.push_back(&I);
              }
            } else if (I.getOperand(0) == I.getOperand(1)){
              Constant* result =  ConstantData::getNullValue(I.getType());
              I.replaceAllUsesWith(result);   
              toRemove.push_back(&I);              
            }
            break;
          }
          // check for mul by 1  
          // check for mul by 0  
          case Instruction::Mul:
          case Instruction::FMul:
          {
            if (Constant* constVal = dyn_cast<Constant>(I.getOperand(0))) {
              if (constVal->isOneValue()) {
                I.replaceAllUsesWith(I.getOperand(1));   
                toRemove.push_back(&I);
              } else if (constVal->isZeroValue()) {
                Constant* result =  ConstantData::getNullValue(I.getType());
                I.replaceAllUsesWith(result);   
                toRemove.push_back(&I);
              }
            } else if (Constant* constVal = dyn_cast<Constant>(I.getOperand(1))) {
              if (constVal->isOneValue()) {
                I.replaceAllUsesWith(I.getOperand(0));   
                toRemove.push_back(&I);
              } else if (constVal->isZeroValue()) {
                Constant* result =  ConstantData::getNullValue(I.getType());
                I.replaceAllUsesWith(result);   
                toRemove.push_back(&I);
              }
            }
            break;
          }
          // check for div by 1 and by the same num
          case Instruction::UDiv:
          case Instruction::SDiv:
          case Instruction::FDiv:
          {
            if (Constant* constVal = dyn_cast<Constant>(I.getOperand(1))) {
              if (constVal->isOneValue()) {
                I.replaceAllUsesWith(I.getOperand(0));   
                toRemove.push_back(&I);
              }
            } else if (I.getOperand(0) == I.getOperand(1)){
              Constant* result =  ConstantData::getAllOnesValue(I.getType());
              I.replaceAllUsesWith(result);   
              toRemove.push_back(&I);              
            }
            break;
          }
          default: break;
        }  // end switch for opcode
      }  // end I for loop

      for(Instruction* I : toRemove) {
        // if (I->isSafeToRemove()) {   // not available in LLVM 6
          I->eraseFromParent();
          ++identities;
          wasModifiedRet = true;
        // }
      }      
      return wasModifiedRet;
    }

    bool doConstantFolding(BasicBlock &BB) {
      bool wasModifiedRet = false;
      std::vector<Instruction*> toRemove;

      for (auto& I : BB) {
        // instead of handling every OPcode individually using ConstantExpr::getOPERATION function
        // e.g. ConstantExpr::getAdd(op0, op1)
        // just use ConstantExpr::get(...) function

        if (I.isBinaryOp()) {
          if (Constant* op0 = dyn_cast<Constant>(I.getOperand(0))) {
            if (Constant* op1 = dyn_cast<Constant>(I.getOperand(1))) {
              Constant* result =  ConstantExpr::get(I.getOpcode(), op0, op1);
              I.replaceAllUsesWith(result);   
              toRemove.push_back(&I);
            }
          }
        }
      }  // end I for loop

      for(Instruction* I : toRemove) {
        // if (I->isSafeToRemove()) {   // not available in LLVM 6
          I->eraseFromParent();
          ++folding;
          wasModifiedRet = true;
        // }
      }
      return wasModifiedRet;
    }

    // helper function
    // sets the arg varOpNum
    // returns the constant literal
    ConstantInt* getVariableOperand(unsigned& varOpNum, Instruction& I) {
      ConstantInt* constVal = NULL;

      if (constVal = dyn_cast<ConstantInt>(I.getOperand(0))) {
        varOpNum = 1;
      } else if (constVal = dyn_cast<ConstantInt>(I.getOperand(1))) {
        varOpNum = 0;
      }
      return constVal;
    }

    // 
    bool doStrengthReduction(BasicBlock &BB) {
      bool wasModifiedRet = false;
      std::vector<Instruction*> toRemove;

      // only handle mul and div for ints
      // only handle power of 2 values
      // in theory we could also handle others by using shift+add
      // but keep it simple and only add shift instr
      // use the BinaryOperator::Create() function
      for (auto& I : BB) {
        switch (I.getOpcode()) {
          case Instruction::Mul:
          {
            unsigned varOpNum = 0;
            ConstantInt* constVal = getVariableOperand(varOpNum, I);

            if (!constVal) {
              break;
            }

            APInt val = constVal->getValue();
            if (val.isPowerOf2()) {
              Constant* shiftBy = ConstantInt::get(I.getType(), val.logBase2());
              BinaryOperator* newShiftInstr = BinaryOperator::Create(Instruction::Shl, I.getOperand(varOpNum), shiftBy, Twine(), &I);
              I.replaceAllUsesWith(newShiftInstr);   
              toRemove.push_back(&I);
            }

            break;
          }
          case Instruction::UDiv:
          case Instruction::SDiv:
          {
            if (ConstantInt* constVal = dyn_cast<ConstantInt>(I.getOperand(1))) {
              APInt val = constVal->getValue();
              if (val.isPowerOf2()) {
                Constant* shiftBy = ConstantInt::get(I.getType(), val.logBase2());
                BinaryOperator* newShiftInstr = BinaryOperator::Create(Instruction::AShr, I.getOperand(0), shiftBy, Twine(), &I);
                I.replaceAllUsesWith(newShiftInstr);   
                toRemove.push_back(&I);
              }
            }
            break;
          }
          default: break;
        }  // end switch for opcode
      }  // end I for loop

      for(Instruction* I : toRemove) {
        // if (I->isSafeToRemove()) {   // not available in LLVM 6
          I->eraseFromParent();
          ++reduction;
          wasModifiedRet = true;
        // }
      }      
      return wasModifiedRet;
    }

    bool runOnBasicBlock(BasicBlock &BB) override {
      bool wasModifiedRet = false;

      /*while BB not unchanged redo all opts
      some optimizations might allow chances for additional optimizations*/
      bool BBchanged;
      do {
        BBchanged = false;
        BBchanged |= doConstantFolding(BB);
        BBchanged |= doIdentities(BB);
        BBchanged |= doStrengthReduction(BB);

        if (BBchanged) {
          wasModifiedRet = true;
        }
      } while (BBchanged);



      return wasModifiedRet;
    }

  };
}

// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char LocalOpts::ID = 0;
static RegisterPass<LocalOpts> X("local-opts", "5984: Local Optimizations", false, false);
