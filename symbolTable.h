#pragma once

#include "node.h"

#include <map>
#include <string>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
/*#include <llvm/LLVMContext.h>
#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Module.h>
#include <llvm/Instructions.h>
#include <llvm/Constants.h>*/

class symbolTable {
private:
  std::map<std::string, llvm::Value*> globals;
  std::map<std::string, llvm::Value*> members;
  std::map<std::string, llvm::Value*> args;
  std::map<std::string, llvm::Value*> locals;
  llvm::Value* thisVal;
public:
  symbolTable() {}
  void addGlobal(std::string name, llvm::Value* newVal);
  void addLocal(std::string name, llvm::Value* newVal);
  llvm::Value* startFunction(llvm::Function* func, list* params);
  llvm::Value* operator[](const std::string key);
};
