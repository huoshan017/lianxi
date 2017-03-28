CC = clang++
AR = ar
CCFLAGS = -g -Wall -O0 -std=c++11 
OBJDIR = $(PWD)/obj

TARGET = libproto.a
TARGET_DIR = $(PWD)/lib
TARGET_LIB = $(TARGET_DIR)/$(TARGET)

SRCS = $(wildcard *.cc)
OBJS = $(patsubst %.cc, $(OBJDIR)/%.o, $(SRCS))

all: init $(TARGET_LIB)  
init:
	mkdir -p $(OBJDIR) $(TARGET_DIR)
	sh ./gen_proto.sh

$(TARGET_LIB): $(OBJS) 
	$(AR) -crs $(TARGET_LIB) $^

$(OBJS) : $(OBJDIR)/%.o : %.cc
	$(CC) $(CCFLAGS) -c $< -o $@ -I../servers/libjmy/thirdparty/include

clean:
	rm -fr $(OBJS) $(TARGET_LIB) $(SRCS) *.h

.PHONY:clean
