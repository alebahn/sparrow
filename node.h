#pragma once

#include <string>

class node {
protected:
  node** children;
  std::string data;
  node(const node& copy);
public:
  node(): children(NULL),data("") {}
  node(node** children, std::string data): children(children),data(data) {}
  ~node();
  node& operator=(const node& rhs);
  std::string getData() const;
  node** getChildren() const;
};

class program : public node {
public:
  program(node** children);
};

class list : public node {
public:
  list();
  list(node* elem);
  list(list* head, list* tail);
  list(node* head, list* tail);
};

class statement : public node {
};

class expression : public statement {
public:
  expression():statement() {}
  expression(node** children);
};

class name : public expression {
public:
  name(std::string data);
};

class func_call : public expression {
public:
  func_call(expression* fname, list* args);
};

class term : public expression {
};

class string_term : public term {
public:
  string_term(std::string data);
};

class class_def : public statement {
};

class def : public statement {
public:
  def(std::string fname, list* params, list* body);
};
