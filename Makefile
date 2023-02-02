FLAGS :=
LN_FLAGS :=
CPP_FLAGS :=
C_FLAGS :=
BINARY := FPB
ODIR := build


ifeq ($(OS),Windows_NT)
    CC = /mingw32/bin/i686-w64-mingw32-gcc
    CXX = /mingw32/bin/i686-w64-mingw32-g++
    OBJC = /mingw32/bin/i686-w64-mingw32-g++
    LN = /mingw32/bin/i686-w64-mingw32-g++
    STRIP = strip
    FIND = find src -type f -name
    PKGCONFIG = pkg-config
    BINARY += .exe

    CPP_FLAGS += -D__WINDOWS__
    LN_FLAGS += -mconsole
else
    CC = gcc
    CXX = g++
    OBJC = g++
    LN = g++
    STRIP = strip
    FIND = find src -type f -name
    UNAME_S := $(shell uname -s)

    ifeq ($(OLDGCC),true)
        LN_FLAGS += -lstdc++fs
    endif

    ifeq ($(UNAME_S),Linux)
	PKGCONFIG = pkg-config
        CCFLAGS += -D LINUX
        BITS := $(shell getconf LONG_BIT)
        ifeq ($(DEBUG),true)
            FLAGS += -rdynamic # debug build
        endif
    endif

    ifeq ($(UNAME_S),Darwin)
	PKGCONFIG = $(shell which pkg-config || echo "/usr/local/bin/pkg-config")
	CPP_FILES += $(shell $(FIND) '*.mm')
    endif
endif

SRC_DIRS += $(shell find src -type d)

CPP_FLAGS += -Isrc
CPP_FLAGS += -MMD -MP
CPP_FILES += $(shell find src -type f -name '*.cpp')

C_FLAGS += $(CPP_FLAGS)
C_FILES += $(shell find src -type f -name '*.c')

CPP_FLAGS += --std=c++17

ifeq ($(DEBUG),true)
FLAGS += -Og -g -D_DEBUG # debug build
POSTBUILD =
else
FLAGS += -O3 # release build
POSTBUILD = $(STRIP) $(BINARY)
endif

FLAGS += -pthread

LN_FLAGS += $(SO_FILES)
LN_FLAGS += -lm

OBJ = $(patsubst %,$(ODIR)/%.o,$(CPP_FILES))
OBJ += $(patsubst %,$(ODIR)/%.o,$(C_FILES))
DEP := $(OBJ:.o=.d)

$(BINARY): $(OBJ)
	$(LN) $^ -o $@ $(FLAGS) $(LN_FLAGS)
	echo "$(FLAGS) $(CPP_FLAGS)" | sed 's/^ *//g' | sed 's/  */\n/g' > compile_flags.txt
	$(POSTBUILD)

$(ODIR)/%.cpp.o: %.cpp
	@mkdir -p "$$(dirname "$@")"
	$(CXX) -c $< -o $@ $(FLAGS) $(CPP_FLAGS)

$(ODIR)/%.c.o: %.c
	@mkdir -p "$$(dirname "$@")"
	$(CC) -c $< -o $@ $(FLAGS) $(C_FLAGS)

$(ODIR)/%.mm.o: %.mm
	@mkdir -p "$$(dirname "$@")"
	$(OBJC) -c $< -o $@ $(FLAGS) $(CPP_FLAGS)

.PHONY: clean

clean:
	rm -rf $(ODIR)

-include $(DEP)
