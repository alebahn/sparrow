CC = gcc

# Flags passed to the preprocessor.
CPPFLAGS += `llvm-config --cppflags`

# Flags passed to the C++ compiler.
CXXFLAGS := -g -gdwarf-2 -O0# -Wall -Wextra
CXXFLAGS := $(CXXFLAGS) `llvm-config --cxxflags`

LDFLAGS := `llvm-config --ldflags`
LIBS := `llvm-config --libs`

all: example

.PHONY: sparrow
sparrow:
	$(MAKE) -C src
	cp src/sparrow .

swruntime.o: swruntime.c swruntime.h
	gcc -g -gdwarf-2 -c swruntime.c

swlib.o: swlib.c swruntime.h
	gcc -g -gdwarf-2 -c swlib.c

clean:
	rm -f *.o
	rm -f sparrow
	rm -f *.bc
	rm -f example
	rm -f helloWorld

cleanall: clean
	$(MAKE) -C src clean

example.bc: sparrow example.sw
	./sparrow example.sw
helloWorld.bc: sparrow helloworld.sw
	./sparrow helloworld.sw

$(bitcodes): $(bitcode)

%.o: %.bc
	opt -mem2reg $< | llc -o - | as -o $@


example: bitcodes = node.bc list.bc swruntime.bc swlib.bc
example: bitcode = example.bc
example: example.o node.o list.o swruntime.o swlib.o
	$(eval bitcodes = node.bc list.bc swruntime.bc swlib.bc)
	gcc -g -gdwarf-2 -o example $^ -lm

helloWorld: bitcodes = greeting.bc
helloWorld: bitcode = helloWorld.bc
helloWorld: helloWorld.o greeting.o swruntime.o swlib.o
	gcc -g -gdwarf-2 -o helloWorld $^ -lm
