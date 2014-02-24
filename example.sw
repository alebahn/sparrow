import console

class node {
  value=string
  next=nullNode
  def init(value) {
    .value=value
  }
  def isNull() { false }
  def setNext(node) {
    .next=node
  }
  def print() {
    console.print(value)
    if (!(.next.isnull())) {
      console.print(",")
    }
    .next.print()
  }
  def getNext() {
    .next
  }
  def getValue() {
    .value
  }
}

class nullNode {
  def isNull() { true }
  def setNext(node) {}
  def print() {}
  def getNext() { this }
  def getValue() { string }
}

class list {
  head=nullNode
  def init() {}
  def insert(value) {
    if (.head.isNull()) {
      .head=node.new(value)
    } else {
      oldHead = .head
      .head=node.new(value)
      .head.setNext(oldHead)
    }
  }
  def print() {
    console.print("(")
    .head.print()
    console.println(")")
  }
  def pop() {
    top = head
    .head = .head.getNext()
    top.getValue()
  }
}

class example {
  theList=list
  def menu() {
    this.printMenu()
    opt = console.readln()
    if (opt == "1\n") {
      input = console.readln()
      .theList.insert(input)
    } elseif (opt == "2\n") {
      .theList.insert(42)
    } elseif (opt == "3\n") {
      .theList.print()
    } elseif (opt == "4\n") {
      console.print(.theList.pop())
    }
    if (opt != "5\n") {
      this.menu()
    }
  }
  def printMenu() {
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
