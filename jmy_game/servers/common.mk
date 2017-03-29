CC = clang++
CCFLAGS = -g -Wall -O0 -std=c++11 -DJEMALLOC_NO_DEMANGLE

JMY_DIR = $(TOP_DIR)/libjmy
JMY_LIB = $(JMY_DIR)/lib
COMMON_DIR = $(TOP_DIR)/common
COMMON_LIB = $(COMMON_DIR)/lib
PROTO_DIR = $(TOP_DIR)/../proto
PROTO_LIB = $(PROTO_DIR)/lib
THIRD_DIR = $(JMY_DIR)/thirdparty
THIRD_LIB = $(THIRD_DIR)/lib
ZLOG_LIB = $(THIRD_LIB)/zlog
JEMALLOC_LIB = $(THIRD_LIB)/jemalloc
PROTOBUF_LIB = $(THIRD_LIB)/protobuf
LIBS = -L$(JMY_LIB) -L$(COMMON_LIB) -L$(PROTO_LIB) -L$(ZLOG_LIB) -L$(JEMALLOC_LIB) -L$(PROTOBUF_LIB)
INCLUDES = -I$(TOP_DIR) -I$(JMY_DIR) -I$(COMMON_DIR)
BINDIR = $(TOP_DIR)/bin

OBJ_DIR = $(SERVER_DIR)/obj
EXE = $(SERVER_BINDIR)/$(SERVER_EXE)

SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: init $(EXE)  
init:
	mkdir -p $(OBJ_DIR) $(SERVER_BINDIR)

$(EXE): $(OBJS) 
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS) \
		-ljmy -lcommon -lproto -lboost_system -lboost_thread \
		$(ZLOG_LIB)/libzlog.a \
		$(JEMALLOC_LIB)/libjemalloc.a \
		$(PROTOBUF_LIB)/libprotobuf.a \
		-lpthread 

$(OBJS) : $(OBJ_DIR)/%.o : %.cpp
	$(CC) $(CCFLAGS) -c $< -o $@
clean:
	rm -fr $(OBJS) $(EXE)

.PHONY:clean
