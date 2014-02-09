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

arglist curargs;
membermap members;
typemap locals;

const std::string *pcname;

//std::set<std::string> provides;
//std::set<std::string> expects;

type::type(std::string classname) {

}

void type::expectFunction(std::string fname) {
  expects.insert(fname);
}

void type::provideClass(std::string cname) {
  //TODO:check existance of cname
  for (std::set<std::string>::iterator it = classes[cname].begin(); it != classes[cname].end(); ++it) {
    provides.insert(*it);
  }
}

std::ostream& operator<<(std::ostream& os, const type* value) {
  os << "{provides:[";
  std::string sep = "";
  for (std::set<std::string>::iterator it = value->provides.begin(); it!=value->provides.end(); ++it) {
    os << sep << *it;
    sep = ",";
  }
  os << "],expects:[";
  sep = "";
  for (std::set<std::string>::iterator it = value->expects.begin(); it!=value->expects.end(); ++it) {
    os << sep << *it;
    sep = ",";
  }
  return os << "]}";
}

template <typename k, typename v>
std::ostream& operator<<(std::ostream& os, const std::map<k,v> value) {
  os << "{";
  std::string sep = "";
  for (typename std::map<k,v>::const_iterator it = value.begin(); it!=value.end(); ++it) {
    os << sep << it->first << ":" << it->second;
    sep = ",";
  }
  return os << "}";
}

template <typename k, typename v>
std::ostream& operator<<(std::ostream& os, const std::map<k,v> *value) {
  return os << *value;
}

std::ostream& operator<<(std::ostream& os, const arglist* value) {
  os << "[";
  std::string sep = "";
  for (arglist::const_iterator it = value->begin(); it!=value->end(); ++it) {
    os << sep << *it;
    sep = ",";
  }
  return os << "]";
}

std::ostream& operator<<(std::ostream& os, const std::set<std::string>& value) {
  os << "[";
  std::string sep = "";
  for (std::set<std::string>::const_iterator it = value.begin(); it!= value.end(); ++it) {
    os << sep << *it;
    sep = ",";
  }
  return os << "]";
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
  members[cname] = new typemap();
  inits->prepass();
  body->prepass();
}

type* list::prepass() const {
  for (unsigned i=0; i<size; ++i) {
    children[i]->prepass();
  }
}

type* name::prepass() const {
  typemap::iterator it;
  if (is_member) {
    it = members[*pcname]->find(data);
    if (it!=members[*pcname]->end())
      return it->second;
    return new type();
  }
  it = locals.find(data);
  if (it!=locals.end())
    return it->second;
  it = members[*pcname]->find(data);
  if (it!=members[*pcname]->end())
    return it->second;

  classmap::iterator cit = classes.find(data);
  if (cit!=classes.end())
    return new type(cit->first);

  return /*locals[data] =*/ new type();
}

type* string_term::prepass() const {
  return new type("string");
}

type* int_term::prepass() const {
  return new type("int");
}

type* float_term::prepass() const {
  return new type("float");
}

type* bool_term::prepass() const {
  return new type("bool");
}

type* func_call::prepass() const {
  object->prepass()->expectFunction(fname);

  arglist *callargs = new arglist();
  type* rettype = new type();

  callargs->push_back(rettype);
  for (unsigned i=0, e=args->getSize(); i<e; ++i) {
    callargs->push_back(args->getChild(i)->prepass());
  }
  functions[fname] = callargs;
  return rettype;
}

type* assign::prepass() const {
  if (vname->isMember())
    return (*members[*pcname])[vname->getValue()] = value->prepass();
  else
    return locals[vname->getValue()] = value->prepass();
}

type* static_assign::prepass() const {
  return (*members[*pcname])[vname] = value->prepass();
}

type* this_term::prepass() const {
  return new type(*pcname);
}

type* def::prepass() const {
  std::string fname2 = fname;
  if (fname == "init")
    fname2 = *pcname+"_new";
  curargs.clear();

  for (unsigned i=0, e=params->getSize(); i<e; ++i) {
    curargs.push_back(new type());
  }

  locals.clear();

  body->prepass();
  //TODO: compare to existing functions
  //TODO: add to functions

  classes[*pcname].insert(fname2);

  //TODO: merge function

  functions[fname2] = new arglist(curargs);

  return NULL;
}

type* if_stmnt::prepass() const {
  cond->prepass()->expectFunction("bool_primitive");
  body->prepass();
  return new type();
}
