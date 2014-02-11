import console

class example {
  def main() {
    if (5<7) {
      console.println("good1")
    }
    if (7<=2) {
      console.println("bad1")
    } else {
      console.println("good2")
    }
    if (7==2) {
      console.println("bad2")
    } else {
      console.println("good3")
    }
    if ("bar"!="foo") {
      console.println("good4")
    }
    if ("bar">"foo") {
      console.println("bad3")
    } else {
      console.println("good5")
    }
  }
}
