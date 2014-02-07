#include "node.h"
#include "codegen.h"
#include "symbolTable.h"

#include <iostream>
#include <map>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Analysis/Verifier.h>

using namespace llvm;

void initLib();
void initStatics();
void initMain();

Module* module;
extern program *root;
IRBuilder<> builder(getGlobalContext());

symbolTable symTable;

Value* class_def::genCode() const {
  module = new llvm::Module(cname, getGlobalContext());
  symTable = symbolTable();
  initLib();
  initStatics();
  body->genCode();
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
    int numArgs = functions[fname]->size();

    if (fname == cname+"_new")
      fname = "new";
    Constant* nameStr = ConstantDataArray::getString(getGlobalContext(), fname);//ConstantArray::get(getGlobalContext(), fname);
    GlobalVariable* gNameStr = new GlobalVariable(*module, nameStr->getType(), true, GlobalVariable::InternalLinkage, nameStr,"");
    ArrayRef<Constant*> twoZeros(std::vector<Constant*>(2, ConstantInt::get(Type::getInt8Ty(getGlobalContext()), 0)));
    //Constant* gepStr = ConstantExpr::getGetElementPtr(gNameStr, twoZeros, true);
    Constant* gepStr = ConstantExpr::getPointerCast(gNameStr, Type::getInt8PtrTy(getGlobalContext()));
    vPair.push_back(gepStr);

    std::vector<Type*> arguments(numArgs+1, Type::getInt8PtrTy(getGlobalContext()));
    FunctionType *ft = FunctionType::get(Type::getInt8PtrTy(getGlobalContext()), arguments, false);
    Constant* func = Function::Create(ft, Function::ExternalLinkage, cname+"_"+fname, module);

    vPair.push_back(ConstantExpr::getPointerCast(func, Type::getInt8PtrTy(getGlobalContext())));
    vPairs.push_back(ConstantStruct::get(pairTy, ArrayRef<Constant*>(vPair)));
    vPair.clear();
  }
  Constant* vtab = ConstantArray::get(ArrayType::get(pairTy, classes[cname].size()), ArrayRef<Constant*>(vPairs));
  GlobalVariable *gvtab = new GlobalVariable(*module, vtab->getType(), true, GlobalVariable::InternalLinkage, vtab, cname+"_vtab");

  std::vector<Constant*> v__class;
  v__class.push_back(ConstantExpr::getPointerCast(gvtab, Type::getInt8PtrTy(getGlobalContext())));
  unsigned i=0;
  for (typemap::iterator it = members[cname]->begin(); it!=members[cname]->end(); ++it, ++i) {
    v__class.push_back(ConstantPointerNull::get(Type::getInt8PtrTy(getGlobalContext())));
    symTable.setMemberIndex(it->first,i);
  }
  ArrayType* __classTy = ArrayType::get(Type::getInt8PtrTy(getGlobalContext()), ++i);
  symTable.setClassType(__classTy);

  //Constant* castgvtab = ConstantExpr::getPointerCast(gvtab, Type::getInt8PtrTy(getGlobalContext()));
  //GlobalVariable *__class = new GlobalVariable(*module, castgvtab->getType(), true, GlobalVariable::InternalLinkage, castgvtab, "__"+cname);
  GlobalVariable *__class = new GlobalVariable(*module, __classTy, true, GlobalVariable::InternalLinkage, ConstantArray::get(__classTy, v__class), "__"+cname);
  Constant* cast__class = ConstantExpr::getPointerCast(__class, Type::getInt8PtrTy(getGlobalContext()));

  std::cerr << cname << std::endl;
  symTable.addGlobal(cname, new GlobalVariable(*module, cast__class->getType(), false, GlobalVariable::ExternalLinkage, cast__class, cname));
}

void addClass(std::string cname) {
  symTable.addGlobal(cname, new GlobalVariable(*module, Type::getInt8PtrTy(getGlobalContext()), false, GlobalVariable::ExternalLinkage, 0, cname));
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
    result = module->getFunction(cname+"_new");
  } else {
    result = module->getFunction(cname+"_"+fname);
  }

  Value* thisVal = symTable.startFunction(result, params);

  BasicBlock *bb = BasicBlock::Create(getGlobalContext(), "entry", result);
  builder.SetInsertPoint(bb);

  if (fname == "init") {
    Function *malloc = module->getFunction("malloc");

    Value* size = ConstantExpr::getPointerCast(ConstantExpr::getGetElementPtr(ConstantPointerNull::get(Type::getInt8PtrTy(getGlobalContext())), ConstantInt::get(Type::getInt8Ty(getGlobalContext()), 1)), Type::getInt64Ty(getGlobalContext()));

    std::vector<Value*> mallocArgs;
    mallocArgs.push_back(size);

    Value* newThis = builder.CreateCall(malloc, mallocArgs);
    newThis->setName("this");
    symTable.setThis(newThis);

    Value* object = thisVal;

    Value* base_addr = builder.CreateBitCast(object, Type::getInt8PtrTy(getGlobalContext())->getPointerTo());
    Value* static_seg = builder.CreateLoad(base_addr);
    Value* this_addr = builder.CreateBitCast(newThis, Type::getInt8PtrTy(getGlobalContext())->getPointerTo());
    builder.CreateStore(static_seg, this_addr);
    body->genCode();
    builder.CreateRet(newThis);
  } else {
    thisVal->setName("this");
    builder.CreateRet(body->genCode());
  }

  verifyFunction(*result);

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
  if (is_member)
    return symTable.getMember(data);
  else
    return symTable[data];
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

Value* assign::genCode() const {
  Value* rhs = value->genCode();
  if (vname->isMember()) {
    //rhs->setName(module->getModuleIdentifier() + "." + vname->getValue());
    symTable.addMember(vname->getValue(), rhs);
  } else {
    rhs->setName(vname->getValue());
    symTable.addLocal(vname->getValue(), rhs);
  }
  return rhs;
}

Value* this_term::genCode() const {
  return symTable.getThis();
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
