all: sparrow

sparrow: main.cpp parser.tab.cpp tokens.cpp
	#g++ -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -o sparrow parser.cpp tokens.cpp
	g++ -o sparrow main.cpp parser.tab.cpp tokens.cpp

tokens.cpp: tokens.l parser.tab.hpp
	lex -o tokens.cpp tokens.l

%.hpp %.tab.cpp: %.ypp
	bison -d parser.ypp

clean:
	rm -f parser.tab.cpp parser.tab.hpp tokens.cpp
	rm -f sparrow
