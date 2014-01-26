#pragma once

//#include "node.h"
#include <string>
#include <ostream>
#include <set>

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

void dump_types();
