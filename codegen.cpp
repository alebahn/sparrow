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

void class_def::initLib() const {
  std::vector<Type*> args = std::vector<Type*>(2, Type::getInt8PtrTy(getGlobalContext()));
  FunctionType *ft = FunctionType::get(Type::getInt8PtrTy(getGlobalContext()), args, false);
  Function::Create(ft, Function::ExternalLinkage, "getfunc", module);

  args = std::vector<Type*>(1, Type::getInt8PtrTy(getGlobalContext()));
  ft = FunctionType::get(Type::getInt8PtrTy(getGlobalContext()), args, false);
  Function::Create(ft, Function::ExternalLinkage, "string_stringLiteral", module);

  GlobalVariable* string_vtab = new GlobalVariable(*module, Type::getInt8PtrTy(getGlobalContext()), false, GlobalVariable::ExternalLinkage, 0, "string_vtab");

  args = std::vector<Type*>(1, Type::getInt64Ty(getGlobalContext()));
  ft = FunctionType::get(Type::getInt8PtrTy(getGlobalContext()), args, false);
  Function::Create(ft, Function::ExternalLinkage, "malloc", module);

  root->getImports()->genCode();
  
  std::string cname = module->getModuleIdentifier();
  list* classes = root->getClasses();
  for (unsigned i=0, e=classes->getSize(); i<e; ++i) {
    std::string name = ((class_def*)classes->getChild(i))->getName();
    addClass(name);
  }
}

void class_def::initStatics() const {
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
    //ArrayRef<Constant*> twoZeros(std::vector<Constant*>(2, ConstantInt::get(Type::getInt8Ty(getGlobalContext()), 0)));
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
  for (unsigned e=inits->getSize(); i<e; ++i) {
    static_assign *a = (static_assign*)inits->getChild(i);
    v__class.push_back(a->getValue()->genConst());
    symTable.setMemberIndex(a->getName(), i);
  }

  ArrayType* __classTy = ArrayType::get(Type::getInt8PtrTy(getGlobalContext()), ++i);
  symTable.setClassType(__classTy);

  module->getGlobalVariable(cname)->setInitializer(ConstantExpr::getPointerCast(ConstantArray::get(__classTy, v__class), Type::getInt8PtrTy(getGlobalContext())));
}

void addClass(std::string cname) {
  symTable.addGlobal(cname, new GlobalVariable(*module, Type::getInt8PtrTy(getGlobalContext()), false, GlobalVariable::ExternalLinkage, 0, cname));
}

void class_def::initMain() const {
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

    Value* size = ConstantExpr::getPointerCast(ConstantExpr::getGetElementPtr(ConstantPointerNull::get(Type::getInt8PtrTy(getGlobalContext())), ConstantInt::get(Type::getInt8Ty(getGlobalContext()), members[cname]->size()+1)), Type::getInt64Ty(getGlobalContext()));

    std::vector<Value*> mallocArgs;
    mallocArgs.push_back(size);

    Value* newThis = builder.CreateCall(malloc, mallocArgs);
    newThis->setName("this");

    Value* objTyped = symTable.getThisTyped();
    symTable.setThis(newThis);
    Value* thisTyped = symTable.getThisTyped();

    Value* intZero = ConstantInt::get(Type::getInt32Ty(getGlobalContext()), 0);
    std::vector<Value*> gepIxs;
    for (unsigned i=0; i<members[cname]->size()+1; ++i) {
      gepIxs.push_back(intZero);
      Value* intOffset = ConstantInt::get(Type::getInt32Ty(getGlobalContext()), i);
      gepIxs.push_back(intOffset);
      Value* src_addr = builder.CreateGEP(objTyped, gepIxs);
      Value* src = builder.CreateLoad(src_addr);
      Value* dest_addr = builder.CreateGEP(thisTyped, gepIxs);
      builder.CreateStore(src, dest_addr);
      gepIxs.clear();
    }

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

Constant* name::genConst() const {
  return module->getGlobalVariable(data);
}

Value* string_term::genCode() const {
  return genConst();
}

Constant* string_term::genConst() const {
  Constant* strVtab = module->getGlobalVariable("string_vtab");
  Constant* theStr = ConstantDataArray::getString(getGlobalContext(), data);
  GlobalVariable* gStr = new GlobalVariable(*module, theStr->getType(), true, GlobalVariable::InternalLinkage, theStr,"");
  ArrayType* strTy = ArrayType::get(Type::getInt8PtrTy(getGlobalContext()), 2);
  Constant* stringObj = ConstantArray::get(strTy, (Constant*[2]){strVtab, gStr});
  GlobalVariable* gStrObj = new GlobalVariable(*module, stringObj->getType(), true, GlobalVariable::InternalLinkage, stringObj,"");
  return ConstantExpr::getPointerCast(gStrObj, Type::getInt8PtrTy(getGlobalContext()));
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

Value* static_assign::genCode() const {
  return NULL;
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
