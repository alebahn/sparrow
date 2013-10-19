#pragma once

#include <string>

class node {
protected:
  node** children;
  std::string data;
  node(const node& copy);
public:
  node(): children(NULL),data("") {}
  ~node();
  node& operator=(const node& rhs);
  std::string getData() const;
  node* getChild(int index) const;
  virtual void genCode() const = 0;
};

class list : public node {
public:
  list();
  list(node* elem);
  list(list* head, list* tail);
  list(node* head, list* tail);
  virtual void genCode() const;
};

class statement : public node {
};

class expression : public statement {
public:
  expression():statement() {}
};

class name : public expression {
public:
  name(std::string data);
  virtual void genCode() const;
};

class func_call : public expression {
public:
  func_call(expression* fname, list* args);
  void genCode() const;
};

class term : public expression {
};

class string_term : public term {
public:
  string_term(std::string data);
  void genCode() const;
};

class class_def : public statement {
};

class def : public statement {
public:
  def(std::string fname, list* params, list* body);
  virtual void genCode() const;
};
