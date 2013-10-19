#pragma once

#include <string>

class node {
protected:
  node** children;
  std::string data;
  node(const node& copy);
public:
  node(node** children, std::string data): children(children),data(data) {}
  ~node();
  node& operator=(const node& rhs);
  std::string getData() const;
};

class program : public node {
public:
  program(node** children): node(children,"") {}
};

class list : public node {
public:
  list();
  list(node* elem);
  list(node* head, list* tail);
};

class expression : public node {
protected:
  expression(const node& copy): node(copy) {}
public:
  expression(node** children): node(children,"") {}
};

class name : public expression {
public:
  name(std::string data): expression(node(NULL,data)) {}
};

class func_call : public expression {
public:
  func_call(expression* fname, node* args);
};

class term : public expression {
protected:
  term(const node& copy): expression(copy) {}
};

class string_term : public term {
public:
  string_term(std::string data): term(node(NULL,data)) {}
};

class statement : public node {
};

class class_def : public statement {
};

class def : public statement {
public:
  def(name* fname, node* args, node* body);
};
