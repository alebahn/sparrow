#include "node.h"

program::program(list* impts, list* clss) {
  imports = impts;
  classes = clss;

  imports = new list(new import("string"), imports);
}

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
  unsigned i;

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
  unsigned i;

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
  for(unsigned i = 0; children[i]!=NULL; ++i) {
    delete children[i];
  }
  delete [] children;
}


