#pragma once

#include <string>
#include <llvm/Value.h>

class node {
public:
  node() {}
  virtual llvm::Value* genCode() const=0;
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
  inline node* getChild(unsigned index) const { return children[index]; }
  inline unsigned getSize() const { return size; }
};

class statement : public node {
};

class expression : public statement {
public:
  expression():statement() {}
};

class name : public expression {
private:
  std::string data;
public:
  name(std::string data):data(data) {}
  virtual llvm::Value* genCode() const;
};

class func_call : public expression {
private:
  expression* object;
  std::string fname;
  list* args;
public:
  func_call(expression* object, std::string fname, list* args): object(object),fname(fname),args(args) {}
  llvm::Value* genCode() const;
};

class term : public expression {
};

class string_term : public term {
private:
  std::string data;
public:
  string_term(std::string data):data(data) {}
  llvm::Value* genCode() const;
};

class class_def : public statement {
private:
  std::string cname;
  list *body;
public:
  class_def(std::string cname, list* body):cname(cname), body(body) {}
  virtual llvm::Value* genCode() const;
};

class def : public statement {
private:
  std::string fname;
  list *params,*body;
public:
  def(std::string fname, list* params, list* body):fname(fname), params(params), body(body) {}
  virtual llvm::Value* genCode() const;
};
