import console
class greeting {
  message = "Hello "
  test = greeting //this only tests assigning yourself
  def init() {

    .message = "World!\n"
  }
  /* testing comment */
  def toString() {
    message
  }
}
class example {
  test = greeting
  def sayHiWith(saying) {
    console.print(saying)
  }
  def main() {
    this.sayHiWith(test)
    newGreeting = greeting.new()
    console.print(newGreeting)
    0
  }
}
