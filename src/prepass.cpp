#include "node.h"
#include "prepass.h"

#include <iostream>
#include <algorithm>

using namespace llvm;

classmap classes;
funcmap functions;

std::map<std::string, int> argids;
arglist *curargs;
membermap members;
typemap locals;

const std::string *pcname;

provides::provides(std::string cname) {
  data = new std::set<std::string>(classes[cname]);
  is_anything = false;
}

void provides::add(provides* parent) {
  parents.insert(parent);
  if (!parent->is_anything)
    is_anything = false;
}

const std::set<std::string>* provides::compile() const {
  std::set<std::string>* result = new std::set<std::string>();
  std::set<std::string> resultcopy;
  if (data->size())
    *result=*data;
  else
    for (std::set<provides*>::iterator it=parents.begin(); it!=parents.end(); ++it) {
      const std::set<std::string> *parent_funcs = (*it)->compile();
      resultcopy = *result;
      set_intersection(parent_funcs->begin(),parent_funcs->end(),
          resultcopy.begin(),resultcopy.end(),
          std::inserter(*result,result->begin()));
      delete parent_funcs;
    }
  return result;
}

const std::set<std::string>* expects::compile() const {
  std::set<std::string> *result = new std::set<std::string>();
  result->insert(data->begin(),data->end());
  for (std::set<expects*>::iterator it=parents.begin(); it!=parents.end(); ++it) {
    const std::set<std::string> *parent_funcs = (*it)->compile();
    result->insert(parent_funcs->begin(),parent_funcs->end());
    delete parent_funcs;
  }
  return result;
}

void type::merge(type* other) {
  other->expec->add(expec);
  prov->add(other->prov);
}

std::ostream& operator<<(std::ostream& os, const provides* value) {
  if (value->is_anything) {
    os << "anything";
  } else {
    os << "[";
    const std::set<std::string> *funcs = value->compile();
    std::string sep = "";
    for (std::set<std::string>::iterator it = funcs->begin(); it!=funcs->end(); ++it) {
      os << sep << *it;
      sep = ",";
    }
    os << "]";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const expects* value) {
  const std::set<std::string> *funcs = value->compile();
  os << "[";
  std::string sep = "";
  for (std::set<std::string>::iterator it = funcs->begin(); it!=funcs->end(); ++it) {
    os << sep << *it;
    sep = ",";
  }
  return os << "]";
}

std::ostream& operator<<(std::ostream& os, const type* value) {
  os << "{provides:" << value->prov;
  os << ",expects:" << value->expec;
  return os << "}";
}

template <typename k, typename v>
std::ostream& operator<<(std::ostream& os, const std::map<k,v> value) {
  os << "{";
  std::string sep = "";
  for (typename std::map<k,v>::const_iterator it = value.begin(); it!=value.end(); ++it) {
    os << sep << it->first << ":" << it->second;
    sep = ",\n";
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
  std::cerr << "{classes:\n" << classes << ",\n\nfunctions:\n" << functions << "}\n" << std::endl << "members:" << members << std::endl;
}

type* program::prepass() const {
  imports->prepass();
  for (int i=0; i<classes->getSize(); ++i) {
    ((class_def*)classes->getChild(i))->genFuncList();
  }
  return classes->prepass();
}

type* import::prepass() const {
}

void class_def::genFuncList() const {
  classes[cname] = std::set<std::string>();
  members[cname] = new typemap();
  for (int i=0; i<body->getSize(); ++i) {
    std::string fname = ((def*)body->getChild(i))->getName();
    if (fname == "init")
      fname = cname+"_new";
    classes[cname].insert(fname);
  }
}

type* class_def::prepass() const {
  pcname = &cname;
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
  std::map<std::string,int>::iterator ait = argids.find(data);
  if (ait!=argids.end())
    return (*curargs)[ait->second];
  it = members[*pcname]->find(data);
  if (it!=members[*pcname]->end())
    return it->second;

  classmap::iterator cit = classes.find(data);
  if (cit!=classes.end())
    return new type(cit->first);

  return new type();
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
  std::string fname2 = fname;
  type* rettype;

  if (fname == "new") {
    fname2 = ((name*)object)->getValue()+"_new";
    rettype = new type(((name*)object)->getValue());
  } else
    rettype = new type();

  object->prepass()->expectFunction(fname2);

  arglist *callargs = new arglist();

  callargs->push_back(rettype);
  for (unsigned i=0, e=args->getSize(); i<e; ++i) {
    callargs->push_back(args->getChild(i)->prepass());
  }
  functions[fname2] = callargs;
  return rettype;
}

type* assign::prepass() const {
  type* record;
  type* lhType = value->prepass();
  if (vname->isMember()) {
    record = (*members[*pcname])[vname->getValue()]; 
    if (record==NULL)
      return (*members[*pcname])[vname->getValue()] = lhType;
    else {
      record->merge(lhType);
      return record;
    }
  } else {
    record = locals[vname->getValue()];
    if (record==NULL)
      return locals[vname->getValue()] = lhType;
    else {
      record->merge(lhType);
      return record;
    }
  }
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
  argids.clear();

  funcmap::iterator it = functions.find(fname2);
  bool declared = (it!=functions.end());
  if (declared)
    curargs=it->second;
  else {
    functions[fname2] = curargs = new arglist();
    curargs->push_back(new type());
  }
  for (unsigned i=0, e=params->getSize(); i<e; ++i) {
    if (!declared)
      curargs->push_back(new type());
    argids[((name*)params->getChild(i))->getValue()]=i+1;
  }

  locals.clear();

  body->prepass();
  //TODO: compare to existing functions
  //TODO: add to functions

  //TODO: merge function

  return NULL;
}

type* if_stmnt::prepass() const {
  cond->prepass()->expectFunction("bool_primitive");
  if_body->prepass();
  if (else_body)
    else_body->prepass();
  //TODO: merge else types
  return new type();
}
