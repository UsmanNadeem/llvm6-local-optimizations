// 5984 S18 Assignment 1: FunctionInfo.cpp
// PID: usman
////////////////////////////////////////////////////////////////////////////////

// Source: http://www.cs.cmu.edu/~15745/15745_assignment1/code/FunctionInfo/FunctionInfo.cpp

#include "unordered_map"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"

#include <iostream>

using namespace llvm;

namespace {
  class FunctionInfo : public FunctionPass {
  public:
    static char ID;
    // struct to keep track of numbers to output
    typedef struct data_t
    {
      bool varArgs = false;
      size_t numArgs = 0;
      size_t numCalls = 0;
      size_t numBB = 0;
      size_t numI = 0;
      size_t numAddSub = 0;
      size_t numMulDiv = 0;
      size_t numBrCond = 0;
      size_t numBrUnCond = 0;
    } Data;

    // map from function name to output data
    std::unordered_map<std::string, Data> funcMap;

    FunctionInfo() : FunctionPass(ID) { }
    ~FunctionInfo() { }

    // We don't modify the program, so we preserve all analyses
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }

    // Do some initialization
    bool doInitialization(Module &M) override {
      outs() << "Name, \tArgs, \tCalls, \tBlocks, \tInsts, \tAdd/Sub, \tMul/Div, \tBr(Cond), \tBr(UnCond)\n";

      // initialize the map
      for (auto& F : M) {
        std::string fName = F.getName();
        funcMap[fName] = Data();
      }

      return false;
    }

    // print all the data
    bool doFinalization(Module &M) {
      for (auto& F : M) {

        std::string fName = F.getName();
        outs() << fName << ", \t";

        if (funcMap[fName].varArgs) {
          outs() << "*" << ", \t";
        } else {
          outs() << funcMap[fName].numArgs << ", \t";
        }

        outs() << funcMap[fName].numCalls << ", \t";
        outs() << funcMap[fName].numBB << ", \t";
        outs() << funcMap[fName].numI << ", \t";
        outs() << funcMap[fName].numAddSub << ", \t";
        outs() << funcMap[fName].numMulDiv << ", \t";
        outs() << funcMap[fName].numBrCond << ", \t";
        outs() << funcMap[fName].numBrUnCond << ", \t";
        outs() << "\n";
      }
      return false;
    }

    // Calculate output values for each function
    // Print them in doFinalization
    bool runOnFunction(Function &F) override {

      std::string fName = F.getName();

      //  number of args
      if (F.isVarArg()) {
        funcMap[fName].varArgs = true;
      } else {
        funcMap[fName].numArgs = F.arg_size();
      }

      // number of BBs
      funcMap[fName].numBB = F.size();
      
      // num instructions
      size_t numI = 0;
      size_t numAddSub = 0;
      size_t numMulDiv = 0;
      size_t numBrCond = 0;
      size_t numBrUnCond = 0;

      for (auto& BB : F) {
        funcMap[fName].numI += BB.size();
        for (auto& I : BB) {
          switch (I.getOpcode()) {
            case Instruction::Add:
            case Instruction::FAdd:
            case Instruction::Sub:
            case Instruction::FSub:
            {
              ++funcMap[fName].numAddSub;
              break;
            }
            case Instruction::Mul:
            case Instruction::FMul:
            case Instruction::UDiv:
            case Instruction::SDiv:
            case Instruction::FDiv:
            {
              ++funcMap[fName].numMulDiv;
              break;
            }
            /*
              Exit Instructions:: Eventhough others could also be considered branches
              I will just look at Br 

              Ret
              Br
              Switch
              IndirectBr
              Invoke
              Resume
              Unreachable
              CleanupRet
              CatchRet
              CatchSwitch
            */
            case Instruction::Br:
            {
              BranchInst* BI = static_cast<BranchInst*>(&I);

              if (BI->isUnconditional()) {
                ++funcMap[fName].numBrUnCond;
              } else if (BI->isConditional()) {
                ++funcMap[fName].numBrCond;
              }
              break;
            }

            // for all call instructions look at the name of the called function
            // update its call count
            case Instruction::Invoke:
            case Instruction::Call:
            {
              std::string calledFName;
              if (InvokeInst* II = dyn_cast<InvokeInst>(&I)) {
                if (Function* calledF = II->getCalledFunction()) {
                  calledFName = calledF->getName();
                }
              } else if (CallInst* CI = dyn_cast<CallInst>(&I)) {
                if (Function* calledF = CI->getCalledFunction()) {
                  calledFName = calledF->getName();
                }
              }

              ++funcMap[calledFName].numCalls;
            }
            default: break;
          }
        }  // end I for loop
      }  // end BB for loop

      return false;
    }
  };
}

// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char FunctionInfo::ID = 0;
static RegisterPass<FunctionInfo> X("function-info", "5984: Function Information", false, false);
