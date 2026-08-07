CROSS_COMPILE ?= 0
DEV_MODE ?=0
LOCAL_LIB_DIR ?= libs
CC := arm-rockchip830-linux-uclibcgnueabihf-gcc
CXX := arm-rockchip830-linux-uclibcgnueabihf-g++
# Require an exported environment variable LUCKFOX_LIBS. If it's not set,
# stop early with a helpful error so the user can export it or pass it on
# the make command line (e.g. `make LUCKFOX_LIBS=/path`).
ifeq ($(CROSS_COMPILE),0)
# Native build: use system paths instead of Luckfox paths
LUCKFOX_LIBS := /usr/lib
LOCAL_LIB_DIR := libs_x64
LUCKFOX_INCLUDES := /usr/include
CC := gcc
CXX := g++
endif

ifndef LUCKFOX_LIBS
$(error LUCKFOX_LIBS is not set; export it or run: make LUCKFOX_LIBS=/path/to/libs)
endif
ifndef LUCKFOX_INCLUDES
$(error LUCKFOX_INCLUDES is not set; export it or run: make LUCKFOX_INCLUDES=/path/to/luckfox/usr/include)
endif
LDFLAGS := -L$(LOCAL_LIB_DIR) -L$(LUCKFOX_LIBS) -Wl,-rpath,$$ORIGIN/libs

# Put your C++ and C sources here (space-separated). Example:
# CPPSRC := agent-core.cpp other.cpp
# CSRC := helper.c
CPPSRC := $(wildcard src/*.cpp)
CSRC := $(wildcard src/*.c)
# Directory mapping
OBJDIR := build/objs
BINDIR := build
INCLUDEDIR := include $(LUCKFOX_INCLUDES)


CPP_SRCS := $(strip $(CPPSRC))
C_SRCS := $(strip $(CSRC))
SRCS := $(CPP_SRCS) $(C_SRCS)

# Map source basenames to object files inside $(OBJDIR)
OBJS := $(patsubst src/%.cpp,$(OBJDIR)/%.o,$(CPP_SRCS)) \
        $(patsubst src/%.c,$(OBJDIR)/%.o,$(C_SRCS))

# Target name: basename of first source (cpp or c). Defaults to 'app'.
MAIN_SRC := $(firstword $(CPP_SRCS) $(C_SRCS))
TARGET_NAME := bacnet_test
TARGET := $(TARGET_NAME)

CXXFLAGS ?= -O2 -Wall -Wextra -std=c++17 $(addprefix -I,$(INCLUDEDIR))
ifeq ($(DEV_MODE),1)
CXXFLAGS += -DDEV_MODE 
endif
CFLAGS ?= -O2 -Wall -Wextra $(addprefix -I,$(INCLUDEDIR))
LDFLAGS ?= 
LDLIBS ?= -lssl -lcrypto -lz -lpthread  -lstdc++fs
DBGFLAGS ?= -g -O0 -D_DEBUG
.PHONY: all clean debug 

all: clean $(TARGET)

debug:
	@$(MAKE) $(TARGET) CXXFLAGS="$(CXXFLAGS) $(DBGFLAGS)"

# Ensure directories exist
$(BINDIR) $(OBJDIR):
	mkdir -p $@

# Link executable from all objects
$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

# Compile C++ sources into build/objs/<basename>.o
$(OBJDIR)/%.o: src/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile C sources into build/objs/<basename>.o
$(OBJDIR)/%.o: src/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BINDIR)/*
	rm -f $(TARGET_NAME)



# Show simple status
print:
	@echo "CPPSRC: $(CPPSRC)"
	@echo "CSRC: $(CSRC)"
	@echo "OBJDIR: $(OBJDIR)"
	@echo "OBJS: $(OBJS)"
	@echo "TARGET: $(TARGET)"