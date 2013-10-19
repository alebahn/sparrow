objects = parser.tab.o tokens.o node.o codegen.o
headers = codegen.h node.h
CC = g++
CXX = g++

GTEST_DIR = ../gtest-1.6.0
USER_DIR = .
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
		$(GTEST_DIR)/include/gtest/internal/*.h
TCPPFLAGS += -I$(GTEST_DIR)/include
TCXXFLAGS += -g -Wall -Wextra -lm

# Flags passed to the preprocessor.
CPPFLAGS += `llvm-config --cppflags`

# Flags passed to the C++ compiler.
CXXFLAGS := -g# -Wall -Wextra
CXXFLAGS := $(CXXFLAGS) `llvm-config --cxxflags`

LDFLAGS := `llvm-config --ldflags`
LIBS := `llvm-config --libs`

all: sparrow

test: tests
	./tests

sparrow: $(objects)
	$(CC) $(CXXFLAGS) $(objects) $(LIBS) $(LDFLAGS) -o sparrow

%.o: %.cpp $(headers)
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c $<

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
	$(CXX) $(TCPPFLAGS) -I$(GTEST_DIR) $(TCXXFLAGS) -c \
	  $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(TCPPFLAGS) -I$(GTEST_DIR) $(TCXXFLAGS) -c \
	  $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^


# Builds a test.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.

tests : node.o test_node.o codegen.o gtest_main.a
	$(CXX) $(TCPPFLAGS) $(TCXXFLAGS) $^ -lpthread -o $@

test_node.o : test_node.cpp node.h $(GTEST_HEADERS)
	$(CXX) $(TCPPFLAGS) $(TCXXFLAGS) -c test_node.cpp
