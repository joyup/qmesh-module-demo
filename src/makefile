#
# 'make'        build executable file 'main'
# 'make clean'  removes all .o and executable files
#
CPU ?= X86
ARCH ?= 64Bit
CJSON_IN ?= 1
COMPILE_PREX = 
ifeq ($(CPU), arm)
COMPILE_PREX = /home/danny/share/toolchain/bin/arm-linux-gnueabihf-
endif
ifeq ($(CPU), rk3308)
COMPILE_PREX = /home/danny/share/toolchain_3308/bin/aarch64-rockchip-linux-gnu-
endif
# define the Cpp compiler to use
CXX = $(COMPILE_PREX)g++
CC = $(COMPILE_PREX)gcc
LD = $(COMPILE_PREX)ld
AR = $(COMPILE_PREX)ar

# define any compile-time flags
CFLAGS := -Wall -g -O2 -fPIC
CXXFLAGS := -std=c++17 -Wall -Wextra -g -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-sign-compare -Wno-unused-variable

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS =

# define output directory
OUTPUT	:= ../lib

# define source directory
SRC		:= .

# define include directory
INCLUDE	:= .

# define lib directory
LIB		:= $(CPU)

ifeq ($(CPU), x86)
MAIN	:= libqmesh_$(CPU)_$(ARCH).a
MAIN_SO	:= libqmesh_$(CPU)_$(ARCH).so
else
MAIN	:= libqmesh_$(CPU).a
MAIN_SO	:= libqmesh_$(CPU).so
ifeq ($(CJSON_IN), 0)
MAIN	:= libqmesh_$(CPU)_nocjson.a
MAIN_SO	:= libqmesh_$(CPU)_nocjson.so
endif
endif
SOURCEDIRS	:= $(shell find $(SRC) -type d)
INCLUDEDIRS	:= $(shell find $(INCLUDE) -type d)
LIBDIRS		:= $(shell find $(LIB) -type d)
FIXPATH = $1
RM = rm -f
MD	:= mkdir -p

# define any directories containing header files other than /usr/include
INCLUDES	:= $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))
INCLUDES += -I../

# define the C libs
LIBS		:= $(patsubst %,-L%, $(LIBDIRS:%/=%))
LIBS += -lpthread -lm -lstdc++

# define the C source files
SOURCES_CPP = $(foreach dir,$(SOURCEDIRS),$(wildcard $(dir)/*.cpp))
SOURCES_C = $(foreach dir,$(SOURCEDIRS),$(wildcard $(dir)/*.c))
ifeq ($(CJSON_IN), 0)
SOURCES_C := $(filter-out ./cJSON.c, $(SOURCES_C))
endif

# define the C object files 
OBJECTS_CPP = $(SOURCES_CPP:%.cpp=%.o)
OBJECTS_C = $(SOURCES_C:%.c=%.o)

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

OUTPUTMAIN	:= $(call FIXPATH,$(OUTPUT)/$(MAIN))
OUTPUTMAIN_SO  := $(call FIXPATH,$(OUTPUT)/$(MAIN_SO))

all: $(OUTPUT) $(MAIN)
	@echo Executing 'all' complete!

$(OUTPUT):
	$(MD) $(OUTPUT)

$(MAIN): $(OBJECTS_CPP) $(OBJECTS_C)
	$(AR) rcs $(OUTPUTMAIN) $(OBJECTS_CPP) $(OBJECTS_C)
#	$(CC) -fPIC -shared -o $(OUTPUTMAIN_SO) $(OBJECTS_CPP) $(OBJECTS_C)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)

$(OBJECTS_CPP): %.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ -c $<
 
$(OBJECTS_C): %.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

.PHONY: clean
clean:
	$(RM) $(OUTPUTMAIN)
	$(RM) $(call FIXPATH,$(OBJECTS_CPP))
	$(RM) $(call FIXPATH,$(OBJECTS_C))
	@echo Cleanup complete!

run: all
	./$(OUTPUTMAIN)
	@echo Executing 'run: all' complete!