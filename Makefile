all: sparrow

sparrow: parser.tab.o tokens.o
	#g++ -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -o sparrow parser.cpp tokens.cpp
	g++ -g -o sparrow parser.tab.o tokens.o

%.o: %.cpp
	g++ -c -g $<

tokens.cpp: tokens.l parser.tab.hpp
	flex++ -o tokens.cpp tokens.l

%.tab.hpp %.tab.cpp: %.ypp
	bison -d $<

clean:
	rm -f parser.tab.cpp parser.tab.hpp tokens.cpp
	rm -f *.o
	rm -f sparrow
