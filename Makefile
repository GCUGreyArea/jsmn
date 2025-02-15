NAME 	 = jsmn

CFLAGS   = -std=c11 -Wall -g -fPIC
CXXFLAGS = -std=c++17 -Wall -g -fPIC

CC=gcc
CXX=g++

# Automated code gerneation. Don't fuck with it 
# unless you know what your doing!
TARGET  = lib$(NAME).so
TEST	= test_$(NAME)

# Yacc file go first because they generat headers
YACSRC = $(patsubst %.y,%.tab.c,$(wildcard src/*.y))
LEXSRC = $(patsubst %.l,%.lex.c,$(wildcard src/*.l))
CXXSRC = $(wildcard src/*.cpp)
CSRC   = $(wildcard src/*.c)

# YACC goes first!
OBJ := $(patsubst %.tab.c,build/%.tab.o,$(YACSRC))
OBJ += $(patsubst %.lex.c,build/%.lex.o,$(LEXSRC))
OBJ += $(patsubst %.cpp,build/%.o,$(CXXSRC))
OBJ += $(patsubst %.c,build/%.o,$(CSRC))

# Test code
TESTCXXSRC := $(wildcard test/*.cpp)
TESTCSRC   := $(wildcard test/*.c)
TESTOBJ	   := $(patsubst %.cpp,build/%.o,$(TESTCXXSRC))
TESTOBJ	   += $(patsubst %.c,build/%.o,$(TESTCSRC))

BUILDTARGET = build/$(TARGET)
TESTTARGET  = build/$(TEST)

# Force rebuild of flex and bison files each time
all: $(BUILDTARGET)
# 	rm -f src/*.tab.* src/*.lex.*

test: $(BUILDTARGET) $(TESTTARGET)
	$(TESTTARGET)

# Dynamic Build Rules
$(BUILDTARGET) : build $(OBJ)
	$(CXX) $(CXXFLAGS) -fPIC -Lbuild -Isrc $(OBJ) -shared  -o $(BUILDTARGET)

$(TESTTARGET): build $(TESTOBJ)
	$(CXX) $(CXXFLAGS) -Lbuild -Isrc -Itest $(TESTOBJ) -ljsmn -lgtest -lpthread -lglog -o $(TESTTARGET) -Wl,-rpath,build

cmd_example: $(BUILDTARGET)
	g++ -std=c++17 -g -Wall -Isrc -c examples/cmd_example.cpp -o cmd_example.o
	g++ -std=c++17 -g -Lbuild -o build/cmd_example cmd_example.o -ljsmn -Wl,-rpath,build
	rm -f cmd_example.o

jsondump: $(BUILDTARGET)
	g++ -std=c++17 -g -Wall -Isrc -c examples/jsondump.cpp -o jsondump.o
	g++ -std=c++17 -g -Lbuild -o jsondump jsondump.o -ljsmn -Wl,-rpath,build
	rm -f jsondump.o

simple: $(BUILDTARGET)
	g++ -std=c++17 -g -Wall -Isrc -c examples/simple.cpp -o simple.o
	g++ -std=c++17 -g -Lbuild -o simple simple.o -ljsmn -Wl,-rpath,build
	rm -f simple.o

build:
	mkdir -p build/src
	mkdir -p build/test

src/%.tab.c: src/*.y
	bison -d $< -o $@

src/%.lex.c: src/%.l
	flex -o $@ $<

build/%.o: %.c
	$(CC) -c $(CFLAGS) -Lbuild -Isrc -Itest $< -o $@

build/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -Lbuild -Isrc -Itest -c $< -o $@

clean:
	rm -rf build cmd_example jsondump simple test.bin tests src/*.tab.h 

.PRECIOUS: *.tab.c 

