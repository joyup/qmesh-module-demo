#
# 'make'        build executable file 'main'
# 'make clean'  removes all .o and executable files
#
CPU ?= X86
ARCH ?= 64Bit
SDKVER ?= 0.7.8
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

# define any compile-time flags
CFLAGS := -Wall -g -O2
CXXFLAGS := -std=c++17 -Wall -Wextra -g -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-sign-compare -Wno-unused-variable

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS =

# define output directory
OUTPUT	:= output

# define source directory
SRC		:= .

# define include directory
INCLUDE	:= . src

# define lib directory
LIB		:= lib

MAIN	:= qmesh_test
SOURCEDIRS	:= $(SRC)
INCLUDEDIRS	:= $(INCLUDE)
LIBDIRS		:= $(LIB)
FIXPATH = $1
RM = rm -f
MD	:= mkdir -p

# define any directories containing header files other than /usr/include
INCLUDES	:= $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))

# define the C libs
LIBS		:= $(patsubst %,-L%, $(LIBDIRS:%/=%))
ifeq ($(CPU), x86)
LIBS += -lqmesh_$(CPU)_$(ARCH)
else
LIBS += -lqmesh_$(CPU)
endif
LIBS += -lpthread -lm -lstdc++

# define the C source files
SOURCES_CPP = $(foreach dir,$(SOURCEDIRS),$(wildcard $(dir)/*.cpp))
# SOURCES_C = $(foreach dir,$(SOURCEDIRS),$(wildcard $(dir)/*.c))
SOURCES_C = qmesh_demo.c

# define the C object files 
OBJECTS_CPP = $(SOURCES_CPP:%.cpp=%.o)
OBJECTS_C = $(SOURCES_C:%.c=%.o)

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

OUTPUTMAIN	:= $(call FIXPATH,$(OUTPUT)/$(MAIN))

all: $(OUTPUT) $(MAIN)
	@echo Executing 'all' complete!

$(OUTPUT):
	$(MD) $(OUTPUT)

$(MAIN): $(OBJECTS_CPP) $(OBJECTS_C)
	$(CC) -o $(OUTPUTMAIN) $(OBJECTS_CPP) $(OBJECTS_C) $(LFLAGS) $(LIBS) 

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

package:
	tar -czvf qmesh_sdk_$(SDKVER).tar.gz ./lib/* cJSON.h makefile qmesh_demo.c qmesh_sdk.h device_upgrade.h
	@echo Executing 'package' complete!