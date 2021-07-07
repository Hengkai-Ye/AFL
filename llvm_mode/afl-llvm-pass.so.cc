/*
  Copyright 2015 Google LLC All rights reserved.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at:

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

/*
   american fuzzy lop - LLVM-mode instrumentation pass
   ---------------------------------------------------

   Written by Laszlo Szekeres <lszekeres@google.com> and
              Michal Zalewski <lcamtuf@google.com>

   LLVM integration design comes from Laszlo Szekeres. C bits copied-and-pasted
   from afl-as.c are Michal's fault.

   This library is plugged into LLVM when invoking clang through afl-clang-fast.
   It tells the compiler to add code roughly equivalent to the bits discussed
   in ../afl-as.h.
*/

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

  GlobalVariable *AFLPrevLoc = new GlobalVariable(
      M, Int32Ty, false, GlobalValue::ExternalLinkage, 0, "__afl_prev_loc",
      0, GlobalVariable::GeneralDynamicTLSModel, 0, false);

  /* Instrument all the things! */
  std::map<unsigned long, int> addr_iv; // <addr of BB, instvalue>
  std::map<unsigned long, unsigned int> addr_line; // <addr of BB, line of BB>
  
  std::ofstream outfile;
  //outfile.open("/home/hengkai/Desktop/AFL/llvm_mode/trace/"+std::string(M.getName())+".log", std::ios::out);
  outfile.open("/home/hengkai/Desktop/AFL/llvm_mode/trace.log", std::ios::app);
  int inst_blocks = 0;
  for (auto &F : M){
    for (auto &BB : F) {

      BasicBlock::iterator IP = BB.getFirstInsertionPt();
      IRBuilder<> IRB(&(*IP));
      
      if (AFL_R(100) >= inst_ratio) continue;

      /* Make up cur_loc */

      unsigned int cur_loc = AFL_R(MAP_SIZE);
      //outfile << cur_loc << std::endl;
      ConstantInt *CurLoc = ConstantInt::get(Int32Ty, cur_loc);
    
      /* Get BB id and its cur_loc & print BBinfo*/
      BasicBlock* BBinfo = &BB;
      addr_iv.insert(std::make_pair((unsigned long)&BB, cur_loc));

      unsigned int max_line = 0;
      std::string filename;
      for(BasicBlock::iterator i = BBinfo->begin(), e = BBinfo->end(); i!=e; ++i){
        Instruction* ii = &*i;
        //errs() << *ii << "\n";
        const DebugLoc &location = ii->getDebugLoc();
        if (location){
          if(max_line < location.getLine()){
            max_line = location.getLine();
          }
        }       
      }

      addr_line.insert(std::make_pair((unsigned long)&BB, max_line));

      for (BasicBlock *Pred : predecessors(BBinfo)){
        unsigned int branch_id = (addr_iv.find((unsigned long)Pred)->second >> 1) xor cur_loc;
        unsigned int src_line = addr_line.find((unsigned long)Pred)->second;
        unsigned int dst_line = addr_line.find((unsigned long)BBinfo)->second;
        errs() << "FLAG:" << branch_id << ":" << src_line << ":" << dst_line << ":" << M.getName() << "\n";
        outfile << "FLAG:" << std::to_string(branch_id) << ":" << std::to_string(src_line) << ":" << std::to_string(dst_line) << ":" << std::string(M.getName()) << std::endl;
        //outfile << "FLAG:" << std::to_string(branch_id) << ":" << addr_iv.find((unsigned long)Pred)->second << ":" << cur_loc << std::endl;
        //errs() << addr_line.find((unsigned long)Pred)->second << "\n";
      }
  

      /*
      //BB.printAsOperand(errs(), false);
      BB.InstValue = cur_loc;
      errs() << "BB-id:" << BB.InstValue << "\n";
      BasicBlock* BBinfo = &BB;

      unsigned int max_line = 0;
      std::string filename;
      for(BasicBlock::iterator i = BBinfo->begin(), e = BBinfo->end(); i!=e; ++i){
        Instruction* ii = &*i;
        //errs() << *ii << "\n"; 
        const DebugLoc &location = ii->getDebugLoc();
        if (location){
          if(max_line < location.getLine()){
            max_line = location.getLine();
          }
        }       
      }
      errs() << "BB-line:" << max_line << "\n";
      errs() << "BB-pred";
      for (BasicBlock *Pred : predecessors(BBinfo)){
        errs() << ":" << Pred->InstValue;
      }
      errs()<<"\n\n";
      */
      
      /* Load prev_loc */

      LoadInst *PrevLoc = IRB.CreateLoad(AFLPrevLoc);
      PrevLoc->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
      Value *PrevLocCasted = IRB.CreateZExt(PrevLoc, IRB.getInt32Ty());
      /* Load SHM pointer */

      LoadInst *MapPtr = IRB.CreateLoad(AFLMapPtr);
      MapPtr->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
      Value *MapPtrIdx =
          IRB.CreateGEP(MapPtr, IRB.CreateXor(PrevLocCasted, CurLoc));

      /* Update bitmap */

      LoadInst *Counter = IRB.CreateLoad(MapPtrIdx);
      Counter->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
      Value *Incr = IRB.CreateAdd(Counter, ConstantInt::get(Int8Ty, 1));
      IRB.CreateStore(Incr, MapPtrIdx)
          ->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));

      /* Set prev_loc to cur_loc >> 1 */

      StoreInst *Store =
          IRB.CreateStore(ConstantInt::get(Int32Ty, cur_loc >> 1), AFLPrevLoc);
      Store->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));

      inst_blocks++;

    }
  //outfile.close();
  }
  outfile.close();
  /* Say something nice. */

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
