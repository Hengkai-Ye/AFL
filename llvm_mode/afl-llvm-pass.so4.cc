#define AFL_LLVM_PASS

#include "../config.h"
#include "../debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <map>
#include <utility>
#include <fstream>
#include <iostream>
#include "llvm/IR/CFG.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

  class AFLCoverage : public ModulePass {

    public:

      static char ID;
      AFLCoverage() : ModulePass(ID) { }

      bool runOnModule(Module &M) override;

      // StringRef getPassName() const override {
      //  return "American Fuzzy Lop Instrumentation";
      // }

  };

}

char AFLCoverage::ID = 0;

bool AFLCoverage::runOnModule(Module &M) {

  LLVMContext &C = M.getContext();

  IntegerType *Int8Ty  = IntegerType::getInt8Ty(C);
  IntegerType *Int32Ty = IntegerType::getInt32Ty(C);

  /* Show a banner */

  char be_quiet = 0;

  if (isatty(2) && !getenv("AFL_QUIET")) {

    SAYF(cCYA "afl-llvm-pass " cBRI VERSION cRST " by <lszekeres@google.com>\n");

  } else be_quiet = 1;

  /* Decide instrumentation ratio */

  char* inst_ratio_str = getenv("AFL_INST_RATIO");
  unsigned int inst_ratio = 100;

  if (inst_ratio_str) {

    if (sscanf(inst_ratio_str, "%u", &inst_ratio) != 1 || !inst_ratio ||
        inst_ratio > 100)
      FATAL("Bad value of AFL_INST_RATIO (must be between 1 and 100)");

  }

  /* Get globals for the SHM region and the previous location. Note that
     __afl_prev_loc is thread-local. */

  GlobalVariable *AFLMapPtr =
      new GlobalVariable(M, PointerType::get(Int8Ty, 0), false,
                         GlobalValue::ExternalLinkage, 0, "__afl_area_ptr");
  
  unsigned int Counter = 0;

  std::map<unsigned long, unsigned int> addr_src; // <addr of BB, line of BB>
  std::map<unsigned long, unsigned int> addr_dst;
  std::ofstream outfile;
  outfile.open("/home/hengkai/Desktop/AFL/llvm_mode/trace.log", std::ios::app);
  int inst_blocks = 0;
  int inst_icmp = 0;
  for (auto &F : M){
    for (auto &BB : F){
      /* Get BB info*/
      unsigned int start_line = 0;
      unsigned int end_line = 0;
      unsigned int flag = 1;
      BasicBlock* BBinfo = &BB;
      Value* BrReceive;
      Value* IcmpReceive;
      
      for(BasicBlock::iterator i = BBinfo->begin(), e = BBinfo->end(); i!=e; ++i){
          Instruction* ii = &*i;
          if(ii->getOpcode() == Instruction::Br){
              BrReceive = ii->getOperand(0);
          }
      }
      for(BasicBlock::iterator i = BBinfo->begin(), e = BBinfo->end(); i!=e; ++i){
          Instruction* ii = &*i;
          if(ii->getOpcode() == Instruction::ICmp){
              IcmpReceive = ii;
              if(IcmpReceive != BrReceive){
                  i++;
                  Instruction* ii_select = &*i;
                  IRBuilder<> IRB(&(*ii_select));
                  ConstantInt *SelectCounter1 = ConstantInt::get(Int32Ty, Counter);
                  Counter++;
                  ConstantInt *SelectCounter2 = ConstantInt::get(Int32Ty, Counter);
                  Counter++;
                  Value* SelectReceive =  IRB.CreateSelect(IcmpReceive, SelectCounter1, SelectCounter2);
                  
                  /* Load SHM pointer */
                  LoadInst *MapPtr = IRB.CreateLoad(AFLMapPtr);
                  MapPtr->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
                  Value *MapPtrIdx =
                  IRB.CreateGEP(MapPtr, SelectReceive);

                  /* Update bitmap*/
                  LoadInst *Counter = IRB.CreateLoad(MapPtrIdx);
                  Counter->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
                  Value *Incr = IRB.CreateAdd(Counter, ConstantInt::get(Int8Ty, 1));
            
                  IRB.CreateStore(Incr, MapPtrIdx)
                      ->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
                
                  inst_icmp++;
                  inst_blocks++;
                  i--;    
              }
          }
        
          const DebugLoc &location = ii->getDebugLoc();
          if (location){
            //errs() << location.getLine() << "\n";
            if(flag){
              start_line = location.getLine();
              if(start_line != 0){
                flag = 0;
              }
            }
            if(end_line < location.getLine()){
              end_line = location.getLine();
            }
          }     
        }
        addr_dst.insert(std::make_pair((unsigned long)&BB, start_line));
        addr_src.insert(std::make_pair((unsigned long)&BB, end_line));   
    }
  }

  /* Instrument all the things! */
  
  for (auto &F : M){
    for (auto &BB : F){

      BasicBlock::iterator IP = BB.getFirstInsertionPt();
      IRBuilder<> IRB(&(*IP));

      if (AFL_R(100) >= inst_ratio) continue;

      Value *Receive;
      BasicBlock* BBinfo = &BB;
      std::string filename;

      unsigned int a = addr_dst.find((unsigned long)BBinfo)->second;
      unsigned int b = addr_src.find((unsigned long)BBinfo)->second;
      errs() << a << ":" << b << "\n";

      /* Instrument a ID for each BB */
      if(pred_size(BBinfo)){

        PHINode *phi = IRB.CreatePHI(Type::getInt32Ty(C), 0);

        for (BasicBlock *Pred : predecessors(BBinfo)){
          /* Instrument a PHINode: ID = [counter, *pred] */
          ConstantInt *IDCounter = ConstantInt::get(Int32Ty, Counter);
          phi->addIncoming(IDCounter, Pred);
          unsigned int src_line = addr_src.find((unsigned long)Pred)->second;
          unsigned int dst_line = addr_dst.find((unsigned long)BBinfo)->second;
          errs() << "----" << src_line << ":" << dst_line << "\n";
          errs() << Pred << "\n";
          outfile << "FLAG:" << std::to_string(Counter) << ":" << std::to_string(src_line) \
            << ":" << std::to_string(dst_line) << ":" << std::string(M.getName()) << std::endl;
          Counter++;
        }

        Receive = phi;

        /* Load SHM pointer */
        LoadInst *MapPtr = IRB.CreateLoad(AFLMapPtr);
        MapPtr->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
        Value *MapPtrIdx =
            IRB.CreateGEP(MapPtr, Receive);

        /* Update bitmap*/
        LoadInst *Counter = IRB.CreateLoad(MapPtrIdx);
        Counter->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
        Value *Incr = IRB.CreateAdd(Counter, ConstantInt::get(Int8Ty, 1));
        
        IRB.CreateStore(Incr, MapPtrIdx)
            ->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
                      
        inst_blocks++;

      }
      
      for (BasicBlock::iterator i = BBinfo->begin(), e = BBinfo->end(); i!=e; i++){
          Instruction* ii = &*i;
          errs() << *ii << "\n";
        }
        
    }
  }
  
  outfile.close();

  if (!be_quiet) {

    if (!inst_blocks) WARNF("No instrumentation targets found.");
    else OKF("Instrumented %u locations (%s mode, ratio %u%%).",
             inst_blocks, getenv("AFL_HARDEN") ? "hardened" :
             ((getenv("AFL_USE_ASAN") || getenv("AFL_USE_MSAN")) ?
              "ASAN/MSAN" : "non-hardened"), inst_ratio);

  }

  return true;

}

static void registerAFLPass(const PassManagerBuilder &,
                            legacy::PassManagerBase &PM) {

  PM.add(new AFLCoverage());

}


static RegisterStandardPasses RegisterAFLPass(
    PassManagerBuilder::EP_ModuleOptimizerEarly, registerAFLPass);

static RegisterStandardPasses RegisterAFLPass0(
    PassManagerBuilder::EP_EnabledOnOptLevel0, registerAFLPass);