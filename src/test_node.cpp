
#include "node.h"
#include "gtest/gtest.h"

class mockNode : public node {
public:
  mockNode() {}
  llvm::Value* genCode() const { return NULL; }
};

TEST(ListTest, Empty) {
  list *a = new list();
  EXPECT_EQ(NULL, a->getChild(0));
  EXPECT_EQ(0, a->getSize());
  delete a;
}

TEST(ListTest, One) {
  mockNode *a = new mockNode();
  list *b = new list(a);
  EXPECT_EQ(a, b->getChild(0));
  EXPECT_EQ(NULL, b->getChild(1));
  EXPECT_EQ(1, b->getSize());
  delete b;
}

TEST(ListTest, Prepend) {
  list *a;
  mockNode *b,*c,*d;
  b = new mockNode();
  c = new mockNode();
  d = new mockNode();

  a = new list(b);
  EXPECT_EQ(b, a->getChild(0));
  EXPECT_EQ(NULL, a->getChild(1));
  EXPECT_EQ(1, a->getSize());

  a = new list(c, a);
  EXPECT_EQ(c, a->getChild(0));
  EXPECT_EQ(b, a->getChild(1));
  EXPECT_EQ(NULL, a->getChild(2));
  EXPECT_EQ(2, a->getSize());
  
  a = new list(d, a);
  EXPECT_EQ(d, a->getChild(0));
  EXPECT_EQ(c, a->getChild(1));
  EXPECT_EQ(b, a->getChild(2));
  EXPECT_EQ(NULL, a->getChild(3));
  EXPECT_EQ(3, a->getSize());
  
  delete a;
}

TEST(ListTest, join) {
  list *a,*b,*c;
  mockNode *d,*e,*f,*g;
  d = new mockNode();
  e = new mockNode();
  f = new mockNode();
  g = new mockNode();

  a = new list(d,new list(e));
  b = new list(f,new list(g));
  c = new list(a,b);

  EXPECT_EQ(d, c->getChild(0));
  EXPECT_EQ(e, c->getChild(1));
  EXPECT_EQ(f, c->getChild(2));
  EXPECT_EQ(g, c->getChild(3));
  EXPECT_EQ(NULL, c->getChild(4));
  EXPECT_EQ(4, c->getSize());
  
  delete c;
}
