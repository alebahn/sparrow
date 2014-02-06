import console
class greeting {
  def init() {}
  def toString() {
    "Hello World!\n"
  }
}
class example {
  def sayHiWith(saying) {
    console.print(saying)
  }
  def main() {
    newGreeting = greeting.new()
    example.sayHiWith(newGreeting)
  }
}
