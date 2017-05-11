CC = clang++

INCLUDES = -I$(PWD)

CCFLAGS = -g -Wall -O0 -std=c++11 -DJEMALLOC_NO_DEMANGLE -fPIC
OBJDIR = $(PWD)/obj
TARGET_EXE = db_generator
TARGET_DIR = $(PWD)/../bin/db_generator
TARGET = $(TARGET_DIR)/$(TARGET_EXE)

SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SRCS))

all: init $(TARGET)
init:
	mkdir -p $(OBJDIR) $(TARGET_DIR)

$(TARGET): $(OBJS) 
	$(CC) $^ -o $@ $(INCLUDES) $(CCFLAGS)

$(OBJS) : $(OBJDIR)/%.o : %.cpp
	$(CC) $(CCFLAGS) -c $< -o $@ $(INCLUDES)

clean:
	rm -fr $(OBJS) $(TARGET)

.PHONY:clean
