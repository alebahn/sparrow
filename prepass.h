#pragma once

//#include "node.h"
#include <string>
#include <ostream>
#include <set>
#include <map>

class type {
private:
  std::string classname;
  std::set<std::string> functions;
  bool isClass;
public:
  type():isClass(false) {}
  type(std::string classname): classname(classname),isClass(true) {}

  void addFunction(std::string function);
  friend std::ostream& operator<<(std::ostream& os, const type& value);
};

typedef std::map<std::string, type*> typemap;
typedef std::map<std::string, typemap*> funcmap;

extern typemap globals;
extern funcmap functions;
extern typemap members;
extern typemap locals;

void dump_types();
