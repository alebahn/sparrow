import console

class node {
  value=string
  next=null
  def init(value) {
    .value=value
  }
  def isNull() { false }
  def setNext(node) {
    .next=node
  }
  def display() {
    if (value==42) {
      console.print("The Answer")
    } else {
      console.print(value)
    }
    can .next display {
      console.print(",")
      .next.display()
    }
  }
  def getNext() {
    .next
  }
  def getValue() {
    .value
  }
}

class list {
  head=null
  def init() {}
  def insert(value) {
    newHead=node.new(value)
    newHead.setNext(.head)
    .head = newHead
  }
  def display() {
    console.print("(")
    can .head display {
      .head.display()
    }
    console.println(")")
  }
  def pop() {
    top = head
    can top getNext {
      .head = top.getNext()
    }
    can top getValue {
      top.getValue()
    } else {
      ""
    }
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
      .theList.display()
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
    0
  }
}
