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
  std::map<std::string, unsigned> members;
  std::map<std::string, llvm::Value*> args;
  std::map<std::string, llvm::Value*> locals;
  llvm::Value* thisVal;
  llvm::Value* thisTyped;
  llvm::Type* classType;

  void checkThisTyped();
public:
  symbolTable() {}
  void addGlobal(std::string name, llvm::Value* newVal);
  void addMember(std::string name, llvm::Value* newVal);
  void addLocal(std::string name, llvm::Value* newVal);
  void setMemberIndex(std::string name, unsigned index);
  inline void setClassType(llvm::Type* classType) { this->classType = classType; }
  void setThis(llvm::Value* thisVal);
  llvm::Value* startFunction(llvm::Function* func, list* params);
  llvm::Value* operator[](const std::string key);
  llvm::Value* getMember(std::string name);
  inline llvm::Value* getThis() { return thisVal; };
};
