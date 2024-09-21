NAME 	   = jsmn

BUILD	   = build
TESTDIR    = test
BUILD      = build


# Automated code gerneation. Don't fuck with it 
# unless you know what your doing!!!

TARGET = lib$(NAME).so

# Yacc file go first because they generat headers
YACSRC = $(patsubst %.y,%.tab.c,$(wildcard src/*.y))
LEXSRC = $(patsubst %.l,%.lex.c,$(wildcard src/*.l))
CXXSRC = $(wildcard src/*.cpp)
CSRC   = $(wildcard src/*.c)

# YACC goes first!
OBJ := $(patsubst %.tab.c,$(BUILD)/%.tab.o,$(YACSRC))
OBJ += $(patsubst %.lex.c,$(BUILD)/%.lex.o,$(LEXSRC))
OBJ += $(patsubst %.cpp,$(BUILD)/%.o,$(CXXSRC))
OBJ += $(patsubst %.cpp,$(BUILD)/%.o,$(CSRC))
    
CFLAGS   = -std=c11 -fPIC -Wall -g 
CXXFLAGS = -std=c++17 -fPIC -Wall -g 

UNAME := $(shell uname)


# Force rebuild of flex and bison files each time
all: $(TARGET)
	rm -f src/*.tab.h

$(TARGET) : build $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -shared  -o $(BUILD)/$(TARGET)

build:
	mkdir -p "$(BUILD)/src"


src/%.tab.c: src/*.y
	bison -d $< -o $@

src/%.lex.c: src/%.l
	flex -o $@ $<

$(BUILD)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)
