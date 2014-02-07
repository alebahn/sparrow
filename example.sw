import console
class greeting {
  def init() {
    .message = "Hello World!\n"
  }
  def toString() {
    message
  }
}
class example {
  def sayHiWith(saying) {
    console.print(saying)
  }
  def main() {
    newGreeting = greeting.new()
    this.sayHiWith(newGreeting)
  }
}
