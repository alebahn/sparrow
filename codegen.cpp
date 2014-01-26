#include "node.h"
#include "codegen.h"

#include <iostream>
#include <map>

#include <llvm/LLVMContext.h>
#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Module.h>
#include <llvm/Instructions.h>
#include <llvm/Support/IRBuilder.h>

using namespace llvm;

Module* module;
static IRBuilder<> builder(getGlobalContext());
//void node::genCode() const {
//}

std::map<std::string, Value*> symTable;

Value* class_def::genCode() const {
  module = new llvm::Module(cname, getGlobalContext());
  initLib();
  body->genCode();
  return NULL;
}

void initLib() {
  std::vector<Type*> args = std::vector<Type*>(2, Type::getInt8PtrTy(getGlobalContext()));
  FunctionType *ft = FunctionType::get(Type::getInt8PtrTy(getGlobalContext()), args, false);
  Function::Create(ft, Function::ExternalLinkage, "getfunc", module);

  args = std::vector<Type*>(1, Type::getInt8PtrTy(getGlobalContext()));
  ft = FunctionType::get(Type::getInt8PtrTy(getGlobalContext()), args, false);
  Function::Create(ft, Function::ExternalLinkage, "string_stringLiteral", module);

  symTable["console"] = new GlobalVariable(*module, Type::getInt8PtrTy(getGlobalContext()), false, GlobalVariable::ExternalLinkage, 0, "console");
  symTable["string"] = new GlobalVariable(*module, Type::getInt8PtrTy(getGlobalContext()), false, GlobalVariable::ExternalLinkage, 0, "string");
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
  std::map<std::string, Value*>::iterator it = symTable.find(data);
  if (it==symTable.end())
    return NULL;
  return builder.CreateLoad(it->second);
}

Value* string_term::genCode() const {
  Value* strPtr = builder.CreateGlobalStringPtr(data);
  Function *stringLiteralFunc = module->getFunction("string_stringLiteral");

  std::vector<Value*> argsV;
  argsV.push_back(strPtr);
  return builder.CreateCall(stringLiteralFunc, argsV);
}

Value* func_call::genCode() const {
  Function *getFunc = module->getFunction("getfunc");

  Value* vobject = object->genCode();

  std::vector<Value*> getFuncArgs;
  getFuncArgs.push_back(vobject);
  getFuncArgs.push_back(builder.CreateGlobalStringPtr(fname));

  Value* funcAddr = builder.CreateCall(getFunc, getFuncArgs);

  std::vector<Type*> types(1+args->getSize(), Type::getInt8PtrTy(getGlobalContext()));
  FunctionType *ft = FunctionType::get(Type::getVoidTy(getGlobalContext()), ArrayRef<Type*>(types), false);
  PointerType *pft = PointerType::get(ft, 0);

  Value *toCall = builder.CreateBitCast(funcAddr, pft);

  std::vector<Value*> argsV;
  argsV.push_back(vobject);
  for (unsigned i=0, e=args->getSize(); i<e; ++i) {
    argsV.push_back(args->getChild(i)->genCode());
  }
  return builder.CreateCall(toCall, argsV);
}

void genCode(list* head) {
  LLVMContext &Context = getGlobalContext();
  module = new Module("sparrow", Context);
  ((class_def*)head)->genCode();
}
