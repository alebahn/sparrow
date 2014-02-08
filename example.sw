import console
class greeting {
  message = "Hello "
  test = greeting
  def init() {
    .message = "World!\n"
  }
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
  }
}
