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
}

class nullnode {
  def isnull() { true }
  def setnext(node) {}
  def print() {}
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
      .thelist.print()
    }
    if (opt != "3\n") {
      this.menu()
    }
  }
  def printmenu() {
    console.println("--MENU--")
    console.println("1) insert value")
    console.println("2) print list")
    console.println("3) quit")
  }
  def main() {
    this.menu()
  }
}
