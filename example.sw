import console

class node {
  value=string
  next=nullnode
  def init(value) {
    .value=value
  }
  def isnull() { false }
  def setnext(node) {
    .next=node
  }
  def print() {
    console.print(value)
    if (!(.next.isnull())) {
      console.print(",")
    }
    .next.print()
  }
  def getnext() {
    .next
  }
  def getvalue() {
    .value
  }
}

class nullnode {
  def isnull() { true }
  def setnext(node) {}
  def print() {}
  def getnext() { this }
  def getvalue() { string }
}

class list {
  head=nullnode
  def init() {}
  def insert(value) {
    if (.head.isnull()) {
      .head=node.new(value)
    } else {
      oldhead = .head
      .head=node.new(value)
      .head.setnext(oldhead)
    }
  }
  def print() {
    console.print("(")
    .head.print()
    console.println(")")
  }
  def pop() {
    top = head
    .head = .head.getnext()
    top.getvalue()
  }
}

class example {
  thelist=list
  def menu() {
    this.printmenu()
    opt = console.readln()
    if (opt == "1\n") {
      input = console.readln()
      .thelist.insert(input)
    } elseif (opt == "2\n") {
      .thelist.insert(42)
    } elseif (opt == "3\n") {
      .thelist.print()
    } elseif (opt == "4\n") {
      console.print(.thelist.pop())
    }
    if (opt != "5\n") {
      this.menu()
    }
  }
  def printmenu() {
    console.println("--MENU--")
    console.println("1) insert value")
    console.println("2) insert 42")
    console.println("3) print list")
    console.println("4) pop")
    console.println("5) quit")
  }
  def main() {
    this.menu()
  }
}
