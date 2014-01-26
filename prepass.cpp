#include "node.h"
#include "prepass.h"

#include <iostream>
#include <map>

//#include <llvm/LLVMContext.h>
//#include <llvm/Type.h>
//#include <llvm/DerivedTypes.h>
//#include <llvm/Module.h>
//#include <llvm/Instructions.h>
//#include <llvm/Support/IRBuilder.h>

using namespace llvm;

typedef std::map<std::string, type*> typemap;
typedef std::map<std::string, typemap*> funcmap;

typemap globals;
funcmap functions;
typemap members;
typemap locals;

typemap* curr_func;

void type::addFunction(std::string function) {
  functions.insert(function);
}

std::ostream& operator<<(std::ostream& os, const type& value) {
  os << "{ ";
  for (std::set<std::string>::iterator it = value.functions.begin(); it!=value.functions.end(); ++it) {
    os << *it << " ";
  }
  os << "}";
  return os;
}

std::ostream& operator<<(std::ostream& os, const typemap& value) {
  for (typemap::const_iterator it = value.begin(); it!=value.end(); ++it) {
    os << it->first << ":" << it->second << std::endl;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, funcmap& value) {
  for (funcmap::iterator it = value.begin(); it!=value.end(); ++it) {
    os << it->first << "():" << std::endl
      << *it->second;
  }
  return os;
}


void dump_types() {
  std::cerr << globals << functions << members;
}

type* class_def::prepass() const {
  body->prepass();
}

type* list::prepass() const {
  for (unsigned i=0; i<size; ++i) {
    children[i]->prepass();
  }
}

type* name::prepass() const {
  typemap::iterator it = locals.find(data);
  if (it!=locals.end())
    return it->second;
  it = curr_func->find(data);
  if (it!=curr_func->end())
    return it->second;
  it = members.find(data);
  if (it!=members.end())
    return it->second;
  it = globals.find(data);
  if (it!=globals.end())
    return it->second;

  return /*locals[data] =*/ new type();
}

type* string_term::prepass() const {
  return new type("string");
}

type* func_call::prepass() const {
  object->prepass()->addFunction(fname);
  return NULL;
}

type* def::prepass() const {
  curr_func = functions[fname] = new typemap();

  for (unsigned i=0, e=params->getSize(); i<e; ++i) {
    (*curr_func)[((name*)params->getChild(i))->getValue()] = new type();
  }

  locals = typemap();

  body->prepass();

  return NULL;
}
