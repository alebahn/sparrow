import console
class greeting {
  def init() {}
  def toString() {
    "Hello World!"
  }
}
class example {
  def main() {
    console.print(greeting.new())
  }
}
