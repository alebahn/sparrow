
#include "node.h"
#include "gtest/gtest.h"

TEST(ListTest, Empty) {
  list *a = new list();
  EXPECT_EQ(NULL, a->getChildren()[0]);
  delete a;
}

TEST(ListTest, One) {
  node *a = new node();
  list *b = new list(a);
  EXPECT_EQ(a, b->getChildren()[0]);
  EXPECT_EQ(NULL, b->getChildren()[1]);
  delete b;
}

TEST(ListTest, Prepend) {
  list *a;
  node *b,*c,*d;
  b = new node();
  c = new node();
  d = new node();

  a = new list(b);
  EXPECT_EQ(b, a->getChildren()[0]);
  EXPECT_EQ(NULL, a->getChildren()[1]);

  a = new list(c, a);
  EXPECT_EQ(c, a->getChildren()[0]);
  EXPECT_EQ(b, a->getChildren()[1]);
  EXPECT_EQ(NULL, a->getChildren()[2]);
  
  a = new list(d, a);
  EXPECT_EQ(d, a->getChildren()[0]);
  EXPECT_EQ(c, a->getChildren()[1]);
  EXPECT_EQ(b, a->getChildren()[2]);
  EXPECT_EQ(NULL, a->getChildren()[3]);
  
  delete a;
}

TEST(ListTest, join) {
  list *a,*b,*c;
  node *d,*e,*f,*g;
  d = new node();
  e = new node();
  f = new node();
  g = new node();

  a = new list(d,new list(e));
  b = new list(f,new list(g));
  c = new list(a,b);

  EXPECT_EQ(d, c->getChildren()[0]);
  EXPECT_EQ(e, c->getChildren()[1]);
  EXPECT_EQ(f, c->getChildren()[2]);
  EXPECT_EQ(g, c->getChildren()[3]);
  EXPECT_EQ(NULL, c->getChildren()[4]);
  
  delete c;
}
