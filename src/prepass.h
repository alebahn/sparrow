#pragma once

//#include "node.h"
#include <string>
#include <ostream>
#include <set>
#include <map>
#include <vector>

class type {
private:
  std::set<std::string> provides;
  std::set<std::string> expects;
public:
  type() {}
  type(std::string cname);

  void expectFunction(std::string fname);
  void provideClass(std::string cname);
  friend std::ostream& operator<<(std::ostream& os, const type* value);
  //friend std::ostream& operator<<(std::ostream& os, const type& value);
};

typedef std::map<std::string, type*> typemap;
typedef std::vector<type*> arglist;
typedef std::map<std::string, arglist*> funcmap;
typedef std::map<std::string, typemap*> membermap;
typedef std::map<std::string, std::set<std::string> > classmap;

extern classmap classes;
extern funcmap functions;
extern membermap members;
extern typemap locals;

void dump_types();
