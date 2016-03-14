ifeq ($(shell uname), Darwin)
	PLATFORM=mac
	ANT=ant
endif
ifeq ($(shell uname), Linux)
	PLATFORM=linux
endif
$SOURCEDIR = src

CC_FILES := $(wildcard src/*.cc)

OBJ_FILES_CC := $(addprefix obj/,$(notdir $(CC_FILES:.cc=.o)))

CC_FILES_COMMON := $(wildcard src/common/*.cc)

OBJ_FILES_COMMON += $(addprefix obj/,$(notdir $(CC_FILES_COMMON:.cc=.o)))

INC = -Isrc/ -Isrc/common/ -Isrc/common/glm -Isrc/common/AntTweak -Isrc/common/GoogleTest
MAIN = main.cc
OUT = bin/GPUFlocking
OUT_TEST = bin/tests

ifeq ($(PLATFORM),mac)
INC += -F/Library/Frameworks/
LINKS =  -framework OpenGL -framework GLUT -Isrc/common/xml -framework Cocoa -framework SDL2 -L/usr/local/lib -lAntTweakBar
endif

GCC = g++ -Wall -pedantic -std=c++0x

#if Linux
ifeq ($(PLATFORM),linux)
INC += $(shell pkg-config --cflags sdl2)
LINKS = $(shell pkg-config --libs sdl2)
LINKS += -lGL -lGLU -lglut -lm -lXt -lX11 -Wl,-rpath,./lib/AntTweakLinux -Llib/AntTweakLinux -Llib/GoogleTest/Linux -lAntTweakBar -lgtest_main
GCC += -std=gnu++0x -Dnullptr=NULL  -g3
endif


all: $(OBJ_FILES_COMMON) $(OBJ_FILES_CC)
	mkdir -p $(CURDIR)/bin
	$(GCC) obj/*.o $(MAIN) -o $(OUT) $(INC) $(LINKS)

test: $(OBJ_FILES_COMMON) $(OBJ_FILES_CC)
	mkdir -p $(CURDIR)/bin
	$(GCC) obj/*.o $(MAIN) -o $(OUT_TEST) $(INC) $(LINKS) -D_TEST

obj/%.o : src/common/%.cc
	mkdir -p $(CURDIR)/obj
	$(GCC) -c -o $@ $< $(INC)

obj/%.o : src/%.cc
	mkdir -p $(CURDIR)/obj
	$(GCC) -c -o $@ $< $(INC)



clean :
	rm obj/*.o

docs:
	doxygen resources/Doxyfile
