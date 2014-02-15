#include "symbolTable.h"

#include <llvm/IR/IRBuilder.h>

#include <iostream>

using namespace llvm;

extern IRBuilder<> builder;

Value* symbolTable::startFunction(Function* func, list* params) {
  Function::arg_iterator it = func->arg_begin();
  thisVal = it++;
  thisTyped = NULL;

  for (unsigned i = 0; i < params->getSize(); ++it, ++i) {
    it->setName(((name*)params->getChild(i))->getValue());

    // Add arguments to variable symbol table.
    args[((name*)params->getChild(i))->getValue()] = it;
  }
  return thisVal;
}

void symbolTable::addGlobal(std::string name, Value* newVal) {
  if (name[0]=='_')
    name.erase(0,1);
  globals[name] = newVal;
}

void symbolTable::addMember(std::string name, Value* newVal) {
  checkThisTyped();
  std::vector<Value*> idxs;
  idxs.push_back(ConstantInt::get(Type::getInt32Ty(getGlobalContext()), 0));
  idxs.push_back(ConstantInt::get(Type::getInt32Ty(getGlobalContext()), members[name]+1));
  Value* memberPtr = builder.CreateGEP(thisTyped, idxs);
  builder.CreateStore(newVal, memberPtr);
}

void symbolTable::addLocal(std::string name, Value* newVal) {
  Function *curFunc = builder.GetInsertBlock()->getParent();
  IRBuilder<> allocB(&curFunc->getEntryBlock(), curFunc->getEntryBlock().begin());
  std::map<std::string, Value*>::iterator it = locals.find(name);
  if (it==locals.end())
    locals[name] = allocB.CreateAlloca(Type::getInt8PtrTy(getGlobalContext()));
  builder.CreateStore(newVal, locals[name]);
}

void symbolTable::setMemberIndex(std::string name, unsigned index) {
  members[name] = index;
}

void symbolTable::setThis(llvm::Value* thisVal) {
  this->thisVal = thisVal;
  thisTyped = NULL;
}

llvm::Value* symbolTable::operator[](const std::string key) {
  std::map<std::string, Value*>::iterator it = args.find(key);
  if (it!=args.end())
    return it->second;
  it = locals.find(key);
  if (it!=locals.end())
    return builder.CreateLoad(it->second);
  std::map<std::string, unsigned>::iterator it2 = members.find(key);
  if (it2!=members.end())
    return getMember(key);
  it = globals.find(key);
  if (it==globals.end()) {
    std::cerr << "something wrong: " << key << std::endl;
    return NULL;
  }
  return builder.CreateBitCast(it->second, Type::getInt8PtrTy(getGlobalContext()));
}

llvm::Value* symbolTable::getMember(std::string name) {
  checkThisTyped();

  std::vector<Value*> idxs;
  idxs.push_back(ConstantInt::get(Type::getInt32Ty(getGlobalContext()), 0));
  idxs.push_back(ConstantInt::get(Type::getInt32Ty(getGlobalContext()), members[name]+1));
  Value* memberPtr = builder.CreateGEP(thisTyped, idxs);
  return builder.CreateLoad(memberPtr);
}

llvm::Value* symbolTable::getThisTyped() {
  checkThisTyped();
  return thisTyped;
}

void symbolTable::checkThisTyped() {
  if (!thisTyped) {
    thisTyped = builder.CreatePointerCast(thisVal, classType->getPointerTo());
  }
}
