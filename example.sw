import console
class greeting {
  def init() {}
  def toString() {
    "Hello World!\n"
  }
}
class example {
  def main() {
    console.print(greeting.new())
  }
}
