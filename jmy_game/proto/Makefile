CC = g++
AR = ar
CCFLAGS = -g -Wall -O0 -std=c++11  -fPIC
OBJDIR = $(PWD)/obj

TARGET = libproto.a
TARGET_DIR = $(PWD)/lib
TARGET_LIB = $(TARGET_DIR)/$(TARGET)
SRCS_DIR = $(PWD)/src

SRCS = $(wildcard $(SRCS_DIR)/*.cc)
OBJS = $(patsubst $(SRCS_DIR)/%.cc, $(OBJDIR)/%.o, $(SRCS))

all: init $(TARGET_LIB)  
init:
	mkdir -p $(OBJDIR) $(TARGET_DIR)
	sh ./gen_proto.sh

$(TARGET_LIB): $(OBJS) 
	$(AR) -crs $(TARGET_LIB) $^

$(OBJS) : $(OBJDIR)/%.o : $(SRCS_DIR)/%.cc
	$(CC) $(CCFLAGS) -c $< -o $@ -I../servers/thirdparty/include

clean:
	rm -fr $(OBJS) $(TARGET_LIB) $(SRCS) $(SRCS_DIR)/*.h

.PHONY:clean
