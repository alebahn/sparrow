#include "node.h"
#include "codegen.h"

#include <iostream>
#include <map>

#include <llvm/LLVMContext.h>
#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Module.h>
#include <llvm/Instructions.h>
#include <llvm/Constants.h>
#include <llvm/Support/IRBuilder.h>

using namespace llvm;

void initLib();
void initStatics();
void initMain();

Module* module;
extern program *root;
static IRBuilder<> builder(getGlobalContext());

std::map<std::string, Value*> symTable;

Value* class_def::genCode() const {
  module = new llvm::Module(cname, getGlobalContext());
  initLib();
  body->genCode();
  initStatics();
  initMain();
  return NULL;
}

void initLib() {
  std::vector<Type*> args = std::vector<Type*>(2, Type::getInt8PtrTy(getGlobalContext()));
  FunctionType *ft = FunctionType::get(Type::getInt8PtrTy(getGlobalContext()), args, false);
  Function::Create(ft, Function::ExternalLinkage, "getfunc", module);

  args = std::vector<Type*>(1, Type::getInt8PtrTy(getGlobalContext()));
  ft = FunctionType::get(Type::getInt8PtrTy(getGlobalContext()), args, false);
  Function::Create(ft, Function::ExternalLinkage, "string_stringLiteral", module);

  args = std::vector<Type*>(1, Type::getInt64Ty(getGlobalContext()));
  ft = FunctionType::get(Type::getInt8PtrTy(getGlobalContext()), args, false);
  Function::Create(ft, Function::ExternalLinkage, "malloc", module);

  root->getImports()->genCode();
  
  std::string cname = module->getModuleIdentifier();
  list* classes = root->getClasses();
  for (unsigned i=0, e=classes->getSize(); i<e; ++i) {
    std::string name = ((class_def*)classes->getChild(i))->getName();
    if (name != cname) {
      addClass(name);
    }
  }
}

void initStatics() {
  std::vector<Type*> vPairTy(2, Type::getInt8PtrTy(getGlobalContext()));
  StructType* pairTy = StructType::create(ArrayRef<Type*>(vPairTy), "struct.pair");
  std::string cname = module->getModuleIdentifier();

  std::vector<Constant*> vPairs;
  std::vector<Constant*> vPair;
  for (std::set<std::string>::iterator it = classes[cname].begin(); it!=classes[cname].end(); ++it) {
    std::string fname = *it;
    if (fname == cname+"_new")
      fname = "new";
    Constant* nameStr = ConstantArray::get(getGlobalContext(), fname);
    GlobalVariable* gNameStr = new GlobalVariable(*module, nameStr->getType(), true, GlobalVariable::InternalLinkage, nameStr,"");
    Constant* gepStr = ConstantExpr::getGetElementPtr(gNameStr, ConstantInt::get(Type::getInt8Ty(getGlobalContext()), 0), true);
    vPair.push_back(gepStr);
    Constant* func = module->getFunction(cname+"_"+fname);
    vPair.push_back(ConstantExpr::getPointerCast(func, Type::getInt8PtrTy(getGlobalContext())));
    vPairs.push_back(ConstantStruct::get(pairTy, ArrayRef<Constant*>(vPair)));
    vPair.clear();
  }
  Constant* vtab = ConstantArray::get(ArrayType::get(pairTy, classes[cname].size()), ArrayRef<Constant*>(vPairs));
  GlobalVariable *gvtab = new GlobalVariable(*module, vtab->getType(), true, GlobalVariable::InternalLinkage, vtab, cname+"_vtab");
  Constant* castgvtab = ConstantExpr::getPointerCast(gvtab, Type::getInt8PtrTy(getGlobalContext()));
  GlobalVariable *__class = new GlobalVariable(*module, castgvtab->getType(), true, GlobalVariable::InternalLinkage, castgvtab, "__"+cname);
  Constant* cast__class = ConstantExpr::getPointerCast(__class, Type::getInt8PtrTy(getGlobalContext()));

  symTable[cname] = new GlobalVariable(*module, cast__class->getType(), false, GlobalVariable::ExternalLinkage, cast__class, cname);
}

void addClass(std::string cname) {
  symTable[cname] = new GlobalVariable(*module, Type::getInt8PtrTy(getGlobalContext()), false, GlobalVariable::ExternalLinkage, 0, cname);

  //std::vector<Type*> args = std::vector<Type*>(2, Type::getInt8PtrTy(getGlobalContext()));
  //FunctionType *ft = FunctionType::get(Type::getInt8PtrTy(getGlobalContext()), args, false);
  //FunctionType *ft = FunctionType::get(Type::getInt8PtrTy(getGlobalContext()), false);
  //Function::Create(ft, Function::ExternalLinkage, cname+"_new", module);
}

void initMain() {
  std::string cname = module->getModuleIdentifier();
  if (classes[cname].find("main")!=classes[cname].end()) {
    FunctionType *ft = FunctionType::get(Type::getInt32Ty(getGlobalContext()),false);
    Function* main = Function::Create(ft, Function::ExternalLinkage, "main", module);

    BasicBlock *bb = BasicBlock::Create(getGlobalContext(), "entry", main);
    builder.SetInsertPoint(bb);

    std::string cname = module->getModuleIdentifier();
    (new func_call(new name(cname), "main", new list()))->genCode();
    builder.CreateRet(ConstantInt::get(Type::getInt32Ty(getGlobalContext()), 0, true));
  }
}

Value* def::genCode() const {
  std::string cname = module->getModuleIdentifier();
  std::vector<Type*> arguments(params->getSize()+1, Type::getInt8PtrTy(getGlobalContext()));
  FunctionType *ft = FunctionType::get(Type::getInt8PtrTy(getGlobalContext()), arguments, false);
  Function* result;
  if (fname == "init") {
    result = Function::Create(ft, Function::ExternalLinkage, cname+"_new", module);
  } else {
    result = Function::Create(ft, Function::ExternalLinkage, cname+"_"+fname, module);
  }

  BasicBlock *bb = BasicBlock::Create(getGlobalContext(), "entry", result);
  builder.SetInsertPoint(bb);

  if (fname == "init") {
    std::cerr << "here";
    Function *malloc = module->getFunction("malloc");

    Value* size = ConstantExpr::getPointerCast(ConstantExpr::getGetElementPtr(ConstantPointerNull::get(Type::getInt8PtrTy(getGlobalContext())), ConstantInt::get(Type::getInt8Ty(getGlobalContext()), 1)), Type::getInt64Ty(getGlobalContext()));

    std::vector<Value*> mallocArgs;
    mallocArgs.push_back(size);

    Value* newThis = builder.CreateCall(malloc, mallocArgs);

    Function::arg_iterator it = result->arg_begin();
    Value* object = it++;

    for (unsigned i = 0; i < params->getSize(); ++it, ++i) {
      //it->setName(args[i]);

      // Add arguments to variable symbol table.
      //NamedValues[args[i]] = it;
    }
    Value* base_addr = builder.CreateBitCast(object, Type::getInt8PtrTy(getGlobalContext())->getPointerTo());
    Value* static_seg = builder.CreateLoad(base_addr);
    Value* this_addr = builder.CreateBitCast(newThis, Type::getInt8PtrTy(getGlobalContext())->getPointerTo());
    builder.CreateStore(static_seg, this_addr);
    body->genCode();
    builder.CreateRet(newThis);
  } else {
    builder.CreateRet(body->genCode());
  }

  return result;
}

Value* list::genCode() const {
  Value* result = NULL;
  for (unsigned i=0; i<size; ++i) {
    result = children[i]->genCode();
  }
  if (result == NULL) {
    return ConstantPointerNull::get(Type::getInt8PtrTy(getGlobalContext()));
  }
  return result;
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

Value* program::genCode() const {
  classes->genCode();
  return NULL;
}

Value* import::genCode() const {
  addClass(cname);
}

Value* func_call::genCode() const {
  Function *getFunc = module->getFunction("getfunc");

  Value* vobject = builder.CreateBitCast(object->genCode(),Type::getInt8PtrTy(getGlobalContext()));

  std::vector<Value*> getFuncArgs;
  getFuncArgs.push_back(vobject);
  getFuncArgs.push_back(builder.CreateGlobalStringPtr(fname));

  Value* funcAddr = builder.CreateCall(getFunc, getFuncArgs);

  std::vector<Type*> types(1+args->getSize(), Type::getInt8PtrTy(getGlobalContext()));
  FunctionType *ft = FunctionType::get(Type::getInt8PtrTy(getGlobalContext()), ArrayRef<Type*>(types), false);
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
