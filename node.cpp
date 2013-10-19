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

func_call::func_call(expression* fname, node* args):expression(node(NULL,"")) {
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

list::list(node* head, list* tail):node(NULL,"") {
  int length;
  int i;

  for(length = 0; tail->children[length] != NULL; ++length);
  children = new node*[length+1];
  children[0] = head;
  for(i = 1; i<length; ++i) {
    children[i] = tail->children[i-1];
  }
  children[i] = NULL;
  delete tail;
}
