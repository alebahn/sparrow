#pragma once

//#include "node.h"
#include <string>
#include <ostream>
#include <set>
#include <map>
#include <vector>

class type;

class provides {
  friend class type;
private:
  bool is_anything;
  std::set<std::string> *data;
  std::set<provides*> parents;

  provides():is_anything(true),data(new std::set<std::string>()) {}
  provides(std::string cname);
  //void add(std::string cname);
  void add(provides* parent);
  const std::set<std::string>* compile() const;
  friend std::ostream& operator <<(std::ostream& os, const provides* value);
};

class expects {
  friend class type;
private:
  std::set<std::string> *data;
  std::set<expects*> parents;

  expects():data(new std::set<std::string>()) {}
  inline void add(std::string fname) { data->insert(fname); }
  inline void add(expects* parent) { parents.insert(parent); }
  const std::set<std::string>* compile() const;
  friend std::ostream& operator <<(std::ostream& os, const expects* value);
};

class type {
private:
  provides *prov;
  expects *expec;
public:
  type():prov(new provides()),expec(new expects()) {}
  type(std::string cname):prov(new provides(cname)),expec(new expects()) {}

  inline void expectFunction(std::string fname) {expec->add(fname);}
  //inline void provideClass(std::string cname) {prov->add(cname);}
  void merge(type* other);
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
