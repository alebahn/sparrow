import console

class example {
  def menu() {
    this.printmenu()
    opt = console.readln()
    if (opt == "1\n") {
      console.println("moo!")
    } elseif (opt == "2\n") {
      input = console.readln()
      console.print(input)
    }
    if (opt != "3\n") {
      this.menu()
    }
  }
  def printmenu() {
    console.println("--MENU--")
    console.println("1) say moo")
    console.println("2) echo")
    console.println("3) quit")
  }
  def main() {
    this.menu()
  }
}
