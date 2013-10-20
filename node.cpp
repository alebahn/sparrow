#include "node.h"

using namespace std;

list::list() {
  children = new node*[1];
  children[0] = NULL;
  size = 0;
}

list::list(node* elem) {
  children = new node*[2];
  children[0] = elem;
  children[1] = NULL;
  size = 1;
}

list::list(node* head, list* tail) {
  int i;

  size = tail->size+1;
  children = new node*[size+1];
  children[0] = head;
  for(i = 0; i<tail->size; ++i) {
    children[i+1] = tail->children[i];
  }
  children[size] = NULL;

  //just deleting the node would free all the branches.
  //free only the list of children and the pointer
  delete [] tail->children;
  tail->children = NULL;
  delete tail;
}

list::list(list* head, list* tail) {
  int i;

  children = new node*[head->size+tail->size+1];
  for(i = 0; i<head->size; ++i) {
    children[i] = head->children[i];
  }
  for(i = 0; i<tail->size; ++i) {
    children[i+head->size] = tail->children[i];
  }
  size = head->size+tail->size;
  children[size] = NULL;

  //just deleting the node would free all the branches.
  //free only the list of children and the pointer
  delete [] head->children;
  head->children = NULL;
  delete head;
  delete [] tail->children;
  tail->children = NULL;
  delete tail;
}

list::~list() {
  if (children == NULL)
    return;
  for(int i = 0; children[i]!=NULL; ++i) {
    delete children[i];
  }
  delete [] children;
}


node* list::getChild(int index) const {
  return children[index];
}

int list::getSize() const {
  return size;
}
