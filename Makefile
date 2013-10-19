objects = parser.tab.o tokens.o node.o
headers = node.h
CC = g++
CXX = g++

GTEST_DIR = ../gtest-1.6.0
USER_DIR = .
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
		$(GTEST_DIR)/include/gtest/internal/*.h

# Flags passed to the preprocessor.
CPPFLAGS += -I$(GTEST_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -lm

all: sparrow

test: tests
	./tests

sparrow: $(objects)
	#$(CC) -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -o sparrow parser.cpp tokens.cpp
	$(CC) -g -o sparrow $(objects)

%.o: %.cpp $(headers)
	$(CC) -c -g $<

tokens.cpp: tokens.l parser.tab.hpp
	flex++ -o tokens.cpp tokens.l

%.tab.hpp %.tab.cpp: %.ypp
	bison -d $<

clean:
	rm -f parser.tab.cpp parser.tab.hpp tokens.cpp
	rm -f *.o
	rm -f *.a
	rm -f tests
	rm -f sparrow

# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
	  $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
	  $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^


# Builds a test.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.

tests : node.o test_node.o gtest_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -lpthread -o $@

test_node.o : test_node.cpp node.h $(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c test_node.cpp
