#include "node.h"
#include "prepass.h"

#include <iostream>

//#include <llvm/LLVMContext.h>
//#include <llvm/Type.h>
//#include <llvm/DerivedTypes.h>
//#include <llvm/Module.h>
//#include <llvm/Instructions.h>
//#include <llvm/Support/IRBuilder.h>

using namespace llvm;

classmap classes;
funcmap functions;

typemap args;
typemap members;
typemap locals;

typemap* curr_func;
const std::string *pcname;

void type::addFunction(std::string function) {
  functions.insert(function);
}

std::ostream& operator<<(std::ostream& os, const type& value) {
  os << "[";
  std::string sep = "";
  for (std::set<std::string>::iterator it = value.functions.begin(); it!=value.functions.end(); ++it) {
    os << sep << *it;
    sep = ",";
  }
  return os << "]";
}

std::ostream& operator<<(std::ostream& os, const typemap& value) {
  os << "{";
  std::string sep = "";
  for (typemap::const_iterator it = value.begin(); it!=value.end(); ++it) {
    os << sep << it->first << ":" << it->second << ",";
    sep = ",";
  }
  return os << "}";
}

std::ostream& operator<<(std::ostream& os, funcmap& value) {
  os << "{";
  std::string sep = "";
  for (funcmap::iterator it = value.begin(); it!=value.end(); ++it) {
    os << sep << it->first << ":" << *it->second << ",";
    sep = ",";
  }
  return os << "}";
}

std::ostream& operator<<(std::ostream& os, classmap& value) {
  os << "{";
  std::string sep = "";
  for (classmap::iterator it = value.begin(); it!=value.end(); ++it) {
    os << sep << it->first << ":[";
    std::string sep2 = "";
    for (std::set<std::string>::iterator vit = it->second.begin(); vit!= it->second.end(); ++vit) {
      os << sep2 << *vit;
      sep2 = ",";
    }
    os << "]";
    sep = ",";
  }
  return os << "}";
}


void dump_types() {
  std::cerr << "{classes:" << classes << ",functions:" << functions << "}" << std::endl << "members:" << members << std::endl;
}

type* program::prepass() const {
  imports->prepass();
  return classes->prepass();
}

type* import::prepass() const {
}

type* class_def::prepass() const {
  pcname = &cname;
  classes[cname] = std::set<std::string>();
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
  /*it = curr_func->find(data);
  if (it!=curr_func->end())
    return it->second;*/
  it = members.find(data);
  if (it!=members.end())
    return it->second;

  classmap::iterator cit = classes.find(data);
  if (cit!=classes.end())
    return new type(it->first);

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
  args.clear();

  for (unsigned i=0, e=params->getSize(); i<e; ++i) {
    args[((name*)params->getChild(i))->getValue()] = new type();
  }

  locals.clear();

  body->prepass();
  //TODO: compare to existing functions
  //TODO: add to functions

  classes[*pcname].insert(fname);

  return NULL;
}
