CC = clang++
AR = ar
CCFLAGS = -g -Wall -O0 -std=c++11 -DJEMALLOC_NO_DEMANGLE -fPIC

THIRD_DIR = $(PWD)/../thirdparty
THIRD_INC = $(THIRD_DIR)/include
INCLUDES = -I$(PWD) -I$(THIRD_INC)

OBJDIR = $(PWD)/obj

TARGET = libjmy.a
TARGET_DIR = $(PWD)/lib
TARGET_LIB = $(TARGET_DIR)/$(TARGET)

SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SRCS))

all: init $(TARGET_LIB)  
init:
	mkdir -p $(OBJDIR) $(TARGET_DIR)

$(TARGET_LIB): $(OBJS) 
	$(AR) -crs $(TARGET_LIB) $^

$(OBJS) : $(OBJDIR)/%.o : %.cpp
	$(CC) $(CCFLAGS) -c $< -o $@ $(INCLUDES)

clean:
	rm -fr $(OBJS) $(TARGET_LIB)

.PHONY:clean
