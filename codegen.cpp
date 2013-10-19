#include "codegen.h"
#include "node.h"

#include <llvm/LLVMContext.h>
#include <llvm/Module.h>

using namespace llvm;

//void node::genCode() const {
//}

void def::genCode() const {
}

void list::genCode() const {
}

void name::genCode() const {
}

void string_term::genCode() const {
}

void func_call::genCode() const {
}

void genCode(node* head) {
  Module* module;
  LLVMContext &Context = getGlobalContext();
  module = new Module("sparrow", Context);
}
