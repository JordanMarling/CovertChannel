######################
#  Jordans Makefile  #
######################

EXECUTABLE=covert_channel
#PARAMS=-c 192.168.1.34 -d dummy -t covert -a 127.0.0.1 -p 0
#PARAMS= -s 127.0.0.1 -d dummy_server -t covert_server
PARAMS=

CCPP=g++
CCPP_FLAGS=-c -Wall

CASM=nasm
CASM_FLAGS=-f elf64

LDFLAGS=
LIBS=
ASM_SOURCES=$(shell ls | grep ".*\.asm$$")
ASM_OBJECTS=$(ASM_SOURCES:.asm=.ao)
CPP_SOURCES=$(shell ls | grep ".*\.c$$") $(shell ls | grep ".*\.cpp$$")
CPP_OBJECTS=$(CPP_SOURCES:.cpp=.o)
CPP_OBJECTS:=$(CPP_OBJECTS:.c=.o)

#export MAKEFLAGS=-j

all: $(EXECUTABLE)

$(EXECUTABLE): $(ASM_OBJECTS) $(CPP_OBJECTS)
	$(CCPP) $(LDFLAGS) $(ASM_OBJECTS) $(CPP_OBJECTS) -o $@ $(LIBS)

run: $(EXECUTABLE)
	./$(EXECUTABLE) $(PARAMS)

debug: CASM_FLAGS += -g -O0
debug: CCPP_FLAGS += -g -O0
debug: clean $(EXECUTABLE)
	gdb --args $(EXECUTABLE) $(PARAMS)

valgrind: CASM_FLAGS += -g -O0
valgrind: CCPP_FLAGS += -g -O0
valgrind: clean $(EXECUTABLE)
	valgrind --track-origins=yes --leak-check=full --show-possibly-lost=no ./$(EXECUTABLE) $(PARAMS)

clean:
	rm -f $(ASM_OBJECTS) $(CPP_OBJECTS) $(EXECUTABLE)

%.ao: %.asm
	$(CASM) $(CASM_FLAGS) $< -o $@ $(LIBS)

%.o: %.cpp
	$(CCPP) $(CCPP_FLAGS) $< -o $@ $(LIBS)

%.o: %.c
	$(CCPP) $(CCPP_FLAGS) $< -o $@ $(LIBS)
