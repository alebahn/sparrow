#include "node.h"
#include "codegen.h"

#include <iostream>

#include <llvm/LLVMContext.h>
#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>

using namespace llvm;

Module* module;
static IRBuilder<> builder(getGlobalContext());
//void node::genCode() const {
//}

Value* class_def::genCode() const {
  module = new llvm::Module(cname, getGlobalContext());
  initLib();
  body->genCode();
  return NULL;
}

void initLib() {
  std::vector<Type*> string(1, Type::getInt8PtrTy(getGlobalContext()));
  FunctionType *ft = FunctionType::get(Type::getVoidTy(getGlobalContext()), string, false);
  Function *print = Function::Create(ft, Function::ExternalLinkage, "print", module);
}

Value* def::genCode() const {
  //FunctionType *ft = FunctionType::get(Type::getVoidTy(getGlobalContext()),ArrayRef(),false)
  FunctionType *ft = FunctionType::get(Type::getVoidTy(getGlobalContext()),false);
  Function* result;
  result = Function::Create(ft, Function::ExternalLinkage, fname, module);

  BasicBlock *bb = BasicBlock::Create(getGlobalContext(), "entry", result);
  builder.SetInsertPoint(bb);

  body->genCode();

  //bb = builder.GetInsertBlock();
  builder.CreateRetVoid();

  return result;
}

Value* list::genCode() const {
  for (unsigned i=0; i<size; ++i) {
    children[i]->genCode();
  }
  return NULL;
}

Value* name::genCode() const {
  return NULL;
}

Value* string_term::genCode() const {
  return builder.CreateGlobalStringPtr(data);
}

Value* func_call::genCode() const {
  Function *toCall = module->getFunction(fname);
  if (toCall == 0)
    std::cerr << "What the func?" << std::endl;

  if (toCall->arg_size() != args->getSize())
    std::cerr << "wrong # args" << std::endl;

  std::vector<Value*> argsV;
  for (unsigned i=0, e=args->getSize(); i<e; ++i) {
    argsV.push_back(args->getChild(i)->genCode());
  }

  //object->genCode();
  return builder.CreateCall(toCall, argsV);
}

void genCode(list* head) {
  LLVMContext &Context = getGlobalContext();
  module = new Module("sparrow", Context);
  ((class_def*)head)->genCode();
}
