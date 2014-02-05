#include "symbolTable.h"

#include <llvm/IR/IRBuilder.h>

#include <iostream>

using namespace llvm;

extern IRBuilder<> builder;

Value* symbolTable::startFunction(Function* func, list* params) {
  Function::arg_iterator it = func->arg_begin();
  thisVal = it++;

  for (unsigned i = 0; i < params->getSize(); ++it, ++i) {
    it->setName(((name*)params->getChild(i))->getValue());

    // Add arguments to variable symbol table.
    args[((name*)params->getChild(i))->getValue()] = it;
  }
  return thisVal;
}

void symbolTable::addGlobal(std::string name, llvm::Value* newVal) {
  globals[name] = newVal;
}

llvm::Value* symbolTable::operator[](const std::string key) {
  std::map<std::string, Value*>::iterator it = args.find(key);
  if (it!=args.end())
    return it->second;
  it = globals.find(key);
  if (it==globals.end()) {
    std::cerr << "something wrong: " << key << std::endl;
    return NULL;
  }
  return builder.CreateLoad(it->second);
}
