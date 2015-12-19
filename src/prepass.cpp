#include "node.h"
#include "prepass.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>

using namespace llvm;

classmap classes;
funcmap functions;

std::map<std::string, int> argids;
arglist *curargs;
membermap members;
typemap locals;

std::string gcname;
const std::streamsize ALL = std::numeric_limits<std::streamsize>::max();

provides::provides(std::string cname) {
  data = new std::set<std::string>(classes[cname]);
  is_anything = false;
}

void provides::add(provides* parent) {
  parents.insert(parent);
  if (!parent->is_anything && is_anything) {
    is_anything = false;
    const std::set<std::string>* compiled_parent = parent->compile();
    data->insert(compiled_parent->begin(),compiled_parent->end());
    delete compiled_parent;
  }
}

const std::set<std::string>* provides::compile() const {
  std::set<std::string>* result = new std::set<std::string>();
  std::set<std::string> resultcopy;
  if (!data->empty())
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
  if (addFunc)
    result->insert(*addFunc);
  return result;
}
std::ostream& operator<<(std::ostream& os, const std::set<std::string>& value);

const std::set<std::string>* expects::compile() const {
  std::set<std::string> *result = new std::set<std::string>();
  result->insert(data->begin(),data->end());
  for (std::set<expects*>::iterator it=parents.begin(); it!=parents.end(); ++it) {
    const std::set<std::string> *parent_funcs = (*it)->compile();
    result->insert(parent_funcs->begin(),parent_funcs->end());
    delete parent_funcs;
  }
  if (ignoreFunc)
    result->erase(*ignoreFunc);
  return result;
}

type* type::getNull() {
  type* result = new type();
  result->prov->is_anything=false;
  return result;
}

type* type::merge(type* other) {
  other->expec->add(expec);
  prov->add(other->prov);
  return this;
}

std::set<std::string>* type::checkMerge() {
  std::set<std::string>* diff = new std::set<std::string>();
  if (prov->is_anything)
    return diff;
  const std::set<std::string>* provComp = prov->compile();
  const std::set<std::string>* expecComp = expec->compile();
  set_difference(expecComp->begin(), expecComp->end(),
      provComp->begin(), provComp->end(),
      std::inserter(*diff, diff->begin()));
  return diff;
}

void type::canFunc(std::string fname) {
  prov->addFunc = new std::string(fname);
  expec->ignoreFunc = new std::string(fname);
}

void node::check(type* typ) {
  std::set<std::string>* diff = typ->checkMerge();
  if (!diff->empty()) {
    std::string message = "Expected function(s) ";
    std::string sep = "";
    for (std::set<std::string>::const_iterator it = diff->begin(); it!= diff->end(); ++it) {
      message += sep + *it;
      sep = ",";
    }
    printError(message+" not provded.");
  }
  delete diff;
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

std::ostream& operator<<(std::ostream& os, const type* value) {
  os << "{";
  std::string sep = "";
  if (!value->prov->is_anything) {
    os << "provides:[";
    const std::set<std::string> *funcs = value->prov->compile();
    for (std::set<std::string>::iterator it = funcs->begin(); it!=funcs->end(); ++it) {
      os << sep << *it;
      sep = ",";
    }
    os << "]";
    delete funcs;
  }
  const std::set<std::string> *funcs = value->expec->compile();
  if (funcs->size()>0) {
    os << sep << "expects:[";
    sep = "";
    for (std::set<std::string>::iterator it = funcs->begin(); it!=funcs->end(); ++it) {
      os << sep << *it;
      sep = ",";
    }
    os << "]";
    delete funcs;
  }
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


void dump_types() {
  std::cerr << "{classes:\n" << classes << ",\n\nfunctions:\n" << functions << "}\n" << std::endl << "members:" << members << std::endl;
}

type* program::prepass() {
  imports->prepass();
  for (unsigned i=0; i<classes->getSize(); ++i) {
    ((class_def*)classes->getChild(i))->genFuncList();
  }
  return classes->prepass();
}

std::istream& operator>>(std::istream& is, std::set<std::string>& set) {
  is.ignore(ALL,'[');
  std::string list;
  getline(is,list,']');
  std::stringstream lstream(list);
  while (lstream) {
    std::string element;
    getline(lstream,element,',');
    if (element!="") {
      set.insert(element);
    }
  }
  return is;
}

std::istream& operator>>(std::istream& is, classmap& cm) {
  is.ignore(ALL,'{');
  while (is.peek() != '}') {
    std::string cname;
    getline(is,cname,':');
    std::set<std::string> funcs;
    is >> funcs;
    cm[cname]=funcs;
    if (is.peek() != '}')
      is.ignore(ALL,',');
  }
  is.ignore(ALL,'}');
  return is;
}

std::istream& operator>>(std::istream& is, provides& prov) {
  is >> *prov.data;
  if (!prov.data->empty()) {
    prov.is_anything=false;
  }
  return is;
}

std::istream& operator>>(std::istream& is, expects& expec) {
  return is >> *expec.data;
}

std::istream& operator>>(std::istream& is, type& val) {
  std::string label;
  if (is.peek() == 'n') {
    getline(is,label,']');
    is.putback(']');
    if (label=="null") {
      val = *type::getNull();
      return is;
    }
  }
  is.ignore(ALL,'{');
  while (is.peek() != '}') {
    getline(is,label,':');
    if (label=="provides") {
      is >> *val.prov;
    } else if (label=="expects") {
      is >> *val.expec;
    }
    if (is.peek() != '}')
      is.ignore(ALL,',');
  }
  is.ignore(ALL,'}');
  return is;
}

std::istream& operator>>(std::istream& is, arglist& args) {
  unsigned i=0;
  is.ignore(ALL,'[');
  while (is.peek() != ']') {
    type* arg = new type();
    is >> *arg;
    if (is.peek() != ']') {
      is.ignore(ALL,',');
    }
    if (args.size() <= i) {
      args.push_back(arg);
    } else {
      args[i]->prov->add(arg->prov);
      args[i]->expec->add(arg->expec);
    }
    i++;
  }
  is.ignore(ALL,']');
  return is;
}

std::istream& operator>>(std::istream& is, funcmap& fm) {
  is.ignore(ALL,'{');
  while (is.peek() != '}') {
    std::string fname;
    getline(is,fname,':');

    arglist* args;
    funcmap::iterator it = fm.find(fname);
    bool declared = (it!=fm.end());
    if (declared)
      args=it->second;
    else {
      fm[fname] = args = new arglist();
    }

    is >> *args;
    if (is.peek() != '}') {
      is.ignore(ALL,',');
      is.ignore(ALL,'\n');
    }
  }
  is.ignore(ALL,'}');
  return is;
}

type* import::prepass() {
  std::ifstream header((cname+".swh").c_str());
  if (!header) {
    printError("header error:" + cname);
  }
  std::string input;
  header.ignore(ALL,'{');
  getline(header,input,':');
  if (input != "classes") {
    printError("header error:" + cname + " expected classes got " + input);
  }
  header >> classes;
  header.ignore(ALL,',');
  header.ignore(ALL,'f');
  getline(header,input,':');
  if (input != "unctions") {
    printError("header error:" + cname + " expected functions got " + input);
  }
  header >> functions;
  return NULL;
}

void class_def::genFuncList() const {
  classes[cname] = std::set<std::string>();
  members[cname] = new typemap();
  for (unsigned i=0; i<body->getSize(); ++i) {
    std::string fname = ((def*)body->getChild(i))->getName();
    if (fname == "init")
      fname = cname+"_new";
    classes[cname].insert(fname);
  }
}

type* class_def::prepass() {
  gcname = cname;
  inits->prepass();
  body->prepass();
  return NULL;
}

type* list::prepass() {
  type* result;
  for (unsigned i=0; i<size; ++i) {
    result = children[i]->prepass();
  }
  return result;
}

type* name::prepass() {
  typemap::iterator it;
  if (is_member) {
    it = members[gcname]->find(data);
    if (it!=members[gcname]->end())
      return data_type=it->second;
    return data_type=new type();
  }
  it = locals.find(data);
  if (it!=locals.end())
    return data_type=it->second;
  std::map<std::string,int>::iterator ait = argids.find(data);
  if (ait!=argids.end())
    return data_type=(*curargs)[ait->second];
  it = members[gcname]->find(data);
  if (it!=members[gcname]->end())
    return data_type=it->second;

  classmap::iterator cit = classes.find(data);
  if (cit!=classes.end())
    return data_type=new type(cit->first);

  return data_type=new type();
}

type* name::prepassConst() {
  classmap::iterator cit = classes.find(data);
  if (cit!=classes.end())
    return data_type=new type(cit->first);
  printError("class "+data+" not defined.");
  return NULL;  //won't be reached. supress warning.
}

void name::setType(type* newType) {
  typemap::iterator it;
  if (is_member) {
    it = members[gcname]->find(data);
    if (it!=members[gcname]->end()) {
      data_type = it->second = newType;
      return;
    }
    printError(data+" not found");
  }
  it = locals.find(data);
  if (it!=locals.end()) {
    data_type = it->second = newType;
    return;
  }
  std::map<std::string,int>::iterator ait = argids.find(data);
  if (ait!=argids.end()) {
    data_type = (*curargs)[ait->second] = newType;
    return;
  }
  it = members[gcname]->find(data);
  if (it!=members[gcname]->end()) {
    data_type = it->second = newType;
    return;
  }
}

type* null_term::prepass() {
  return type::getNull();
}

type* string_term::prepass() {
  return new type("string");
}

type* int_term::prepass() {
  return new type("int");
}

type* float_term::prepass() {
  return new type("float");
}

type* bool_term::prepass() {
  return new type("bool");
}

type* func_call::prepass() {
  std::string fname2 = fname;
  arglist *callargs;
  type* rettype = new type();

  if (fname == "new")
    fname2 = ((name*)object)->getValue()+"_new";

  funcmap::iterator it = functions.find(fname2);
  bool declared = (it!=functions.end());

  if (declared) {
    callargs = it->second;
    if (callargs->size()!=args->getSize()+1) {
      printError("Number of arguments for "+fname+" does not match existing usage");
    }
  }
  else
    functions[fname2] = callargs = new arglist();

  check(object->prepass()->expectFunction(fname2));

  if (declared) {
    check(rettype->merge((*callargs)[0]));
  }
  else
    callargs->push_back(rettype);

  for (unsigned i=0, e=args->getSize(); i<e; ++i) {
    if (declared) {
      check((*callargs)[i+1]->merge(args->getChild(i)->prepass()));
    }
    else
      callargs->push_back(args->getChild(i)->prepass());
  }

  return rettype;
}

type* assign::prepass() {
  type* record;
  type* rhType = value->prepass();
  if (vname->isMember()) {
    record = (*members[gcname])[vname->getValue()];
    if (record==NULL)
      return (*members[gcname])[vname->getValue()] = rhType;
    else {
      check(rhType->merge(record));
      return record;
    }
  } else {
    record = locals[vname->getValue()];
    if (record==NULL) {
      return locals[vname->getValue()] = rhType;
    }
    else {
      check(record->merge(rhType));
      return record;
    }
  }
}

type* static_assign::prepass() {
  return (*members[gcname])[vname] = value->prepassConst();
}

type* this_term::prepass() {
  return new type(gcname);
}

type* def::prepass() {
  type* rettype;
  type* bodyType;
  std::string fname2 = fname;
  if (fname == "init") {
    fname2 = gcname+"_new";
    rettype = new type(gcname);
  } else {
    rettype = new type();
  }
  argids.clear();

  funcmap::iterator it = functions.find(fname2);
  bool declared = (it!=functions.end());
  if (declared) {
    curargs = it->second;
    if (curargs->size()!=params->getSize()+1) {
      printError("Number of arguments for "+fname+" does not match existing usage");
    }
    check((*curargs)[0]->merge(rettype));
  }
  else {
    functions[fname2] = curargs = new arglist();
    curargs->push_back(rettype);
  }
  for (unsigned i=0, e=params->getSize(); i<e; ++i) {
    if (!declared)
      curargs->push_back(new type());
    argids[((name*)params->getChild(i))->getValue()]=i+1;
  }

  locals.clear();

  bodyType = body->prepass();
  if (fname != "init")
    check(rettype->merge(bodyType));

  return rettype;
}

type* if_stmnt::prepass() {
  type* result = new type();
  check(cond->prepass()->expectFunction("boolPrimitive"));
  check(result->merge(if_body->prepass()));
  if (else_body)
    check(result->merge(else_body->prepass()));
  else {
    delete result;
    return type::getNull();
  }
  return result;
}

type* can_stmnt::prepass() {
  type* result = new type();

  type* varType = vname->prepass();
  type* newVarType = new type();
  check(newVarType->merge(varType));
  newVarType->canFunc(fname);

  vname->setType(newVarType);
  check(result->merge(if_body->prepass()));
  vname->setType(varType);

  if (else_body)
    check(result->merge(else_body->prepass()));
  else {
    delete result;
    return type::getNull();
  }
  return result;
}

type* while_stmnt::prepass() {
  check(cond->prepass()->expectFunction("boolPrimitive"));
  while_body->prepass();
  return type::getNull();
}
