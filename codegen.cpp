#include "codegen.h"
#include "node.h"

#include <llvm/LLVMContext.h>
#include <llvm/Module.h>

using namespace llvm;

Module* module;
//void node::genCode() const {
//}

Value* def::genCode() const {
  Function* result;
//  result = Function::Create(
  return NULL;
}

Value* list::genCode() const {
  return NULL;
}

Value* name::genCode() const {
  return NULL;
}

Value* string_term::genCode() const {
  return NULL;
}

Value* func_call::genCode() const {
  return NULL;
}

void genCode(node* head) {
  LLVMContext &Context = getGlobalContext();
  module = new Module("sparrow", Context);
  head->genCode();
}
