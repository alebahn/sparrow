#include "node.h"
#include "codegen.h"
#include "symbolTable.h"
#include "parser.tab.hpp"

#include <iostream>
#include <map>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>

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
  ft = FunctionType::get(Type::getInt1Ty(getGlobalContext()), args, false);
  Function::Create(ft, Function::ExternalLinkage, "bool_boolPrimitive", module);
  ft = FunctionType::get(Type::getInt32Ty(getGlobalContext()), args, false);
  Function::Create(ft, Function::ExternalLinkage, "int_intPrimitive", module);

  new GlobalVariable(*module, StructType::get(getGlobalContext()), false, GlobalVariable::ExternalLinkage, 0, "string_vtab");
  new GlobalVariable(*module, Type::getInt8PtrTy(getGlobalContext()), false, GlobalVariable::ExternalLinkage, 0, "int_vtab");
  new GlobalVariable(*module, Type::getInt8PtrTy(getGlobalContext()), false, GlobalVariable::ExternalLinkage, 0, "bool_vtab");

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
    Constant* nameStr = ConstantDataArray::getString(getGlobalContext(), fname);
    GlobalVariable* gNameStr = new GlobalVariable(*module, nameStr->getType(), true, GlobalVariable::InternalLinkage, nameStr,"");
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
  ++i;
  std::vector<Type*> v__class_type(i);
  std::fill(v__class_type.begin(),v__class_type.end(),Type::getInt8PtrTy(getGlobalContext()));

  StructType* __classTy = module->getTypeByName(cname);
  __classTy->setBody(v__class_type);
  symTable.setClassType(__classTy);

    
  Constant *statics = ConstantStruct::get(__classTy, ArrayRef<Constant*>(v__class));
  module->getGlobalVariable(cname)->setInitializer(statics);
}

void addClass(std::string cname) {
  StructType* cType = module->getTypeByName(cname);
  if (cType==NULL) cType = StructType::create(getGlobalContext(), cname);
  symTable.addGlobal(cname, new GlobalVariable(*module, cType, false, GlobalVariable::ExternalLinkage, 0, cname));
}

void class_def::initMain() const {
  std::string cname = module->getModuleIdentifier();
  if (classes[cname].find("main")!=classes[cname].end()) {
    FunctionType *ft = FunctionType::get(Type::getInt32Ty(getGlobalContext()),false);
    Function* main = Function::Create(ft, Function::ExternalLinkage, "main", module);

    BasicBlock *bb = BasicBlock::Create(getGlobalContext(), "entry", main);
    builder.SetInsertPoint(bb);

    std::string cname = module->getModuleIdentifier();
    Value* returnVal = (new func_call(new func_call(new name(cname), "main", new list()), "toInt", new list()))->genCode();
    returnVal = builder.CreateCall(module->getFunction("int_intPrimitive"), returnVal);
    builder.CreateRet(returnVal);
  }
}

Value* def::genCode() const {
  std::string cname = module->getModuleIdentifier();
  std::vector<Type*> arguments(params->getSize()+1, Type::getInt8PtrTy(getGlobalContext()));
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

    Constant* size = ConstantExpr::getGetElementPtr(Type::getInt8Ty(getGlobalContext()), ConstantPointerNull::get(Type::getInt8PtrTy(getGlobalContext())), ConstantInt::get(Type::getInt8Ty(getGlobalContext()), members[cname]->size()+1));
    size = ConstantExpr::getBitCast(size,Type::getInt64Ty(getGlobalContext()));

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

Value* if_stmnt::genCond() const {
  return builder.CreateCall(module->getFunction("bool_boolPrimitive"), cond->genCode());
}

Value* can_stmnt::genCond() const {
  module->getFunction("getfunc");
  Value* fPtr = builder.CreateCall(module->getFunction("getfunc"), ArrayRef<Value*>{vname->genCode(),builder.CreateGlobalStringPtr(fname)});
  return builder.CreateICmpNE(builder.CreatePtrToInt(fPtr,Type::getInt64Ty(getGlobalContext())),ConstantInt::get(Type::getInt64Ty(getGlobalContext()), 0));
}

Value* branch_stmnt::genCode() const {
  Value* cond_bool = this->genCond();

  BasicBlock *preBB = builder.GetInsertBlock();
  Function *curFunc = preBB->getParent();
  BasicBlock *thenBB = BasicBlock::Create(getGlobalContext(), "then", curFunc);
  Value *thenVal;
  BasicBlock *elseBB;
  Value *elseVal;
  BasicBlock *ifContBB = BasicBlock::Create(getGlobalContext(), "ifcont");

  if (else_body) {
    elseBB = BasicBlock::Create(getGlobalContext(), "else");
  } else {
    elseBB = ifContBB;
  }

  builder.CreateCondBr(cond_bool, thenBB, elseBB);

  builder.SetInsertPoint(thenBB);
  thenVal = if_body->genCode();
  builder.CreateBr(ifContBB);
  thenBB = builder.GetInsertBlock();

  if (else_body) {
    curFunc->getBasicBlockList().push_back(elseBB);
    builder.SetInsertPoint(elseBB);
    elseVal = else_body->genCode();
    builder.CreateBr(ifContBB);
    elseBB = builder.GetInsertBlock();
  }

  curFunc->getBasicBlockList().push_back(ifContBB);
  builder.SetInsertPoint(ifContBB);
  PHINode* result = builder.CreatePHI(Type::getInt8PtrTy(getGlobalContext()),2);
  result->addIncoming(thenVal, thenBB);
  if (else_body) {
    result->addIncoming(elseVal, elseBB);
  } else {
    result->addIncoming(ConstantPointerNull::get(Type::getInt8PtrTy(getGlobalContext())), preBB);
  }

  return result;
}

Value* while_stmnt::genCode() const {
  Value* cond_bool = builder.CreateCall(module->getFunction("bool_boolPrimitive"), cond->genCode());

  BasicBlock *preBB = builder.GetInsertBlock();
  Function *curFunc = preBB->getParent();
  BasicBlock *bodyBB = BasicBlock::Create(getGlobalContext(), "whilebody", curFunc);
  Value *bodyVal;
  BasicBlock *whileContBB = BasicBlock::Create(getGlobalContext(), "whilecont");

  builder.CreateCondBr(cond_bool, bodyBB, whileContBB);

  builder.SetInsertPoint(bodyBB);
  bodyVal = while_body->genCode();

  cond_bool = builder.CreateCall(module->getFunction("bool_boolPrimitive"), cond->genCode());
  builder.CreateCondBr(cond_bool, bodyBB, whileContBB);
  bodyBB = builder.GetInsertBlock();

  curFunc->getBasicBlockList().push_back(whileContBB);
  builder.SetInsertPoint(whileContBB);
  PHINode* result = builder.CreatePHI(Type::getInt8PtrTy(getGlobalContext()),2);
  result->addIncoming(bodyVal, bodyBB);
  result->addIncoming(ConstantPointerNull::get(Type::getInt8PtrTy(getGlobalContext())), preBB);

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
  //TODO: fix for low level types
  Constant* global = module->getGlobalVariable(data);
  return ConstantExpr::getPointerCast(global, Type::getInt8PtrTy(getGlobalContext()));
}

Constant* null_term::genConst() const {
  return ConstantPointerNull::get(Type::getInt8PtrTy(getGlobalContext()));
}

Constant* string_term::genConst() const {
  Constant* strVtab = module->getGlobalVariable("string_vtab");
  Constant* pStrVtab = ConstantExpr::getPointerCast(strVtab, Type::getInt8PtrTy(getGlobalContext()));
  Constant* theStr = ConstantDataArray::getString(getGlobalContext(), data);
  GlobalVariable* gStr = new GlobalVariable(*module, theStr->getType(), true, GlobalVariable::InternalLinkage, theStr,"");
  Constant* pgStr = ConstantExpr::getPointerCast(gStr, Type::getInt8PtrTy(getGlobalContext()));
  ArrayType* strTy = ArrayType::get(Type::getInt8PtrTy(getGlobalContext()), 2);
  Constant* stringObj = ConstantArray::get(strTy, ArrayRef<Constant*>{pStrVtab, pgStr});
  GlobalVariable* gStrObj = new GlobalVariable(*module, stringObj->getType(), true, GlobalVariable::InternalLinkage, stringObj,"");
  return ConstantExpr::getPointerCast(gStrObj, Type::getInt8PtrTy(getGlobalContext()));
}

Constant* int_term::genConst() const {
  Constant* intVtab = module->getGlobalVariable("int_vtab");
  Constant* pIntVtab = ConstantExpr::getPointerCast(intVtab, Type::getInt8PtrTy(getGlobalContext()));
  Constant* theInt = ConstantInt::get(Type::getInt32Ty(getGlobalContext()), data, true);
  std::vector<Type*> vIntTy;
  vIntTy.push_back(Type::getInt8PtrTy(getGlobalContext()));
  vIntTy.push_back(Type::getInt32Ty(getGlobalContext()));
  StructType* intTy = StructType::create(vIntTy, "struct.int");
  Constant* intObj = ConstantStruct::get(intTy, ArrayRef<Constant*>{pIntVtab, theInt});
  GlobalVariable* gIntObj = new GlobalVariable(*module, intObj->getType(), true, GlobalVariable::InternalLinkage, intObj,"");
  return ConstantExpr::getPointerCast(gIntObj, Type::getInt8PtrTy(getGlobalContext()));
}

Constant* float_term::genConst() const {
  //TODO: implement float
  return NULL;
}

Constant* bool_term::genConst() const {
  Constant* boolVtab = module->getGlobalVariable("bool_vtab");
  Constant* pBoolVtab = ConstantExpr::getPointerCast(boolVtab, Type::getInt8PtrTy(getGlobalContext()));
  Constant* theBool = ConstantInt::get(Type::getInt8Ty(getGlobalContext()), data);
  std::vector<Type*> vBoolTy;
  vBoolTy.push_back(Type::getInt8PtrTy(getGlobalContext()));
  vBoolTy.push_back(Type::getInt8Ty(getGlobalContext()));
  StructType* boolTy = StructType::create(vBoolTy, "struct.bool");
  Constant* boolObj = ConstantStruct::get(boolTy, ArrayRef<Constant*>{pBoolVtab, theBool});
  GlobalVariable* gBoolObj = new GlobalVariable(*module, boolObj->getType(), true, GlobalVariable::InternalLinkage, boolObj,"");
  return ConstantExpr::getPointerCast(gBoolObj, Type::getInt8PtrTy(getGlobalContext()));
}

Value* program::genCode() const {
  classes->genCode();
  return NULL;
}

Value* import::genCode() const {
  addClass(cname);
  return NULL;
}

Value* assign::genCode() const {
  Value* rhs = value->genCode();
  if (vname->isMember()) {
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
