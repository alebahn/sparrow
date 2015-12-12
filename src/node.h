#pragma once

#include "prepass.h"

#include <string>
#include <llvm/IR/Value.h>
#include <llvm/IR/Constants.h>

extern "C" {
  typedef struct YYLTYPE YYLTYPE;
}

class node {
protected:
  YYLTYPE *loc;
public:
  node();
  virtual ~node();
  void setLoc(YYLTYPE loc);
  virtual llvm::Value* genCode() const=0;
  virtual type* prepass()=0;
  void printError(std::string message);
};

class list : public node {
protected:
  unsigned size;
  node** children;
public:
  list();
  ~list();
  list(node* elem);
  list(list* head, list* tail);
  list(node* head, list* tail);
  virtual llvm::Value* genCode() const;
  virtual type* prepass();
  inline node* getChild(unsigned index) const { return children[index]; }
  inline unsigned getSize() const { return size; }
};

class name;

class statement : public node {
};

class expression : public statement {
};

class func_call : public expression {
private:
  expression* object;
  std::string fname;
  list* args;
public:
  func_call(expression* object, std::string fname, list* args): object(object),fname(fname),args(args) {}
  llvm::Value* genCode() const;
  virtual type* prepass();
};

class program: public node{
private:
  list *imports;
  list *classes;
public:
  program(list* imports, list* classes);
  virtual llvm::Value* genCode() const;
  virtual type* prepass();
  inline list* getClasses() const { return classes; }
  inline list* getImports() const { return imports; }
};

class class_def : public statement {
private:
  std::string cname;
  list *inits;
  list *body;
public:
  class_def(std::string cname, list* inits, list* body):cname(cname), inits(inits), body(body) {}
  inline std::string getName() const { return cname; }
  virtual llvm::Value* genCode() const;
  virtual type* prepass();
  void genFuncList() const;
private:
  void initLib() const;
  void initStatics() const;
  void initMain() const;
};

class this_term : public expression {
public:
  this_term() {}
  virtual llvm::Value* genCode() const;
  virtual type* prepass();
};

class import : public statement {
private:
  std::string cname;
public:
  import(std::string cname):cname(cname) {}
  virtual llvm::Value* genCode() const;
  virtual type* prepass();
};

class def : public statement {
private:
  std::string fname;
  list *params,*body;
public:
  def(std::string fname, list* params, list* body):fname(fname), params(params), body(body) {}
  std::string getName() const { return fname; }
  virtual llvm::Value* genCode() const;
  virtual type* prepass();
};

class branch_stmnt : public statement {
private:
  virtual llvm::Value* genCode() const;
protected:
  list *if_body;
  list *else_body;
  virtual llvm::Value* genCond() const=0;
public:
  branch_stmnt(list* if_body, list* else_body):if_body(if_body), else_body(else_body) {}
};

class if_stmnt : public branch_stmnt {
private:
  expression *cond;
public:
  if_stmnt(expression *cond, list* if_body, list* else_body=NULL):cond(cond), branch_stmnt(if_body,else_body) {}
  virtual type* prepass();
  virtual llvm::Value* genCond() const;
};

class can_stmnt : public branch_stmnt {
private:
  name *vname;
  std::string fname;
public:
  can_stmnt(name *vname, std::string fname, list* can_body, list* else_body=NULL):vname(vname), fname(fname), branch_stmnt(can_body,else_body) {}
  virtual type* prepass();
  virtual llvm::Value* genCond() const;
};

class assign : public expression {
private:
  name *vname;
  expression *value;
public:
  assign(name* vname, expression* value):vname(vname), value(value) {}
  virtual llvm::Value* genCode() const;
  virtual type* prepass();
  inline name* getName() const { return vname; }
  inline expression* getValue() const { return value; }
};

class const_expr : public expression {
public:
  virtual llvm::Constant* genConst() const=0;
  virtual llvm::Value* genCode() const { return genConst(); }
  virtual type* prepassConst() { return prepass(); }
};

class static_assign : public statement {
private:
  std::string vname;
  const_expr *value;
public:
  static_assign(std::string vname, const_expr* value):vname(vname), value(value) {}
  virtual llvm::Value* genCode() const;
  virtual type* prepass();
  inline std::string getName() const { return vname; }
  inline const_expr* getValue() const { return value; }
};

class name : public const_expr {
private:
  std::string data;
  bool is_member;
  type* data_type;
public:
  name(std::string data, bool is_member = false):data(data),is_member(is_member) {}
  inline std::string getValue() const { return data; }
  inline bool isMember() const { return is_member; }
  virtual llvm::Value* genCode() const;
  virtual llvm::Constant* genConst() const;
  virtual type* prepass();
  virtual type* prepassConst();
};

class null_term : public const_expr {
public:
  virtual llvm::Constant* genConst() const;
  virtual type* prepass();
};

class string_term : public const_expr {
private:
  std::string data;
public:
  string_term(std::string data);
  virtual llvm::Constant* genConst() const;
  virtual type* prepass();
};

class int_term : public const_expr {
private:
  int data;
public:
  int_term(int data):data(data) {};
  virtual llvm::Constant* genConst() const;
  virtual type* prepass();
};

class float_term : public const_expr {
private:
  double data;
public:
  float_term(double data):data(data) {};
  virtual llvm::Constant* genConst() const;
  virtual type* prepass();
};

class bool_term : public const_expr {
private:
  bool data;
public:
  bool_term(bool data):data(data) {};
  virtual llvm::Constant* genConst() const;
  virtual type* prepass();
};
