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
    example.sayHiWith(greeting.new())
  }
}
