#include "node.h"

using namespace std;

list::list() {
  children = new node*[1];
  children[0] = NULL;
}

list::list(node* elem) {
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

  //just deleting the node would free all the branches.
  //free only the list of children and the pointer
  delete [] tail->children;
  tail->children = NULL;
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
  int size;

  for(size = 0; children[size] != NULL; ++size);
  return size;
}
