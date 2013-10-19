#include "node.h"

using namespace std;

node::node(const node& copy) {
  children = copy.children;
  data = copy.data;
}

node& node::operator=(const node& rhs) {
  throw(this);  //test assumption this will never be used
}

node::~node() {
  delete [] children;
}

string node::getData() const {
  return data;
}

node** node::getChildren() const{
  return children;
}

expression::expression(node** children) {
  this->children = children;
}

name::name(string data) {
  this->data = data;
}

string_term::string_term(string data) {
  this->data = data;
}

func_call::func_call(expression* fname, list* args) {
  children = new node*[3];
  children[0] = fname;
  children[1] = args;
  children[2] = NULL;
}

list::list():node(NULL,"") {
  children = new node*[1];
  children[0] = NULL;
}

list::list(node* elem):node(NULL,"") {
  children = new node*[2];
  children[0] = elem;
  children[1] = NULL;
}

list::list(node* head, list* tail) {
  int length;
  int i;

  for(length = 0; tail->children[length] != NULL; ++length);
  children = new node*[length+2];
  children[0] = head;
  for(i = 0; i<length; ++i) {
    children[i+1] = tail->children[i];
  }
  children[i+1] = NULL;
  delete tail;
}

list::list(list* head, list* tail) {
  int lengtha;
  int lengthb;
  int i;

  for(lengtha = 0; head->children[lengtha] != NULL; ++lengtha);
  for(lengthb = 0; tail->children[lengthb] != NULL; ++lengthb);
  children = new node*[lengtha+lengthb+1];
  for(i = 0; i<lengtha; ++i) {
    children[i] = head->children[i];
  }
  for(i = 0; i<lengthb; ++i) {
    children[i+lengtha] = tail->children[i];
  }
  children[lengtha+lengthb] = NULL;
  delete head;
  delete tail;
}

def::def(string fname, list* params, list* body) {
  data = fname;
  children = new node*[3];
  children[0] = params;
  children[1] = body;
  children[2] = NULL;
}
