all: sparrow

sparrow: main.o parser.tab.o tokens.o
	#g++ -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -o sparrow parser.cpp tokens.cpp
	g++ -o sparrow main.o parser.tab.o tokens.o

%.o: %.cpp
	g++ -c $<

tokens.cpp: tokens.l parser.tab.hpp
	lex -o tokens.cpp tokens.l

%.hpp %.tab.cpp: %.ypp
	bison -d $<

clean:
	rm -f parser.tab.cpp parser.tab.hpp tokens.cpp
	rm -f *.o
	rm -f sparrow
