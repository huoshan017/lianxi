CC = clang++
CCFLAGS = -g -Wall -O0 -std=c++11 -DJEMALLOC_NO_DEMANGLE

JMYDIR = $(TOPDIR)/libjmy
INCLUDES = -I$(TOPDIR) -I$(JMYDIR)
THIRDLIB = $(JMYDIR)/thirdparty/lib
LIBS = -L$(JMYDIR)/lib -L$(TOPDIR)/../proto/lib -L$(THIRDLIB)/zlog -L$(THIRDLIB)/jemalloc -L$(THIRDLIB)/protobuf
BINDIR = $(TOPDIR)/bin

OBJDIR = $(SERVER_DIR)/obj
EXE = $(SERVER_BINDIR)/$(SERVER_EXE)

SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SRCS))

all: init $(EXE)  
init:
	mkdir -p $(OBJDIR) $(SERVER_BINDIR)

$(EXE): $(OBJS) 
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS) -ljmy -lproto -lboost_system -lboost_thread $(THIRDLIB)/zlog/libzlog.a $(THIRDLIB)/jemalloc/libjemalloc.a -lpthread -lprotobuf

$(OBJS) : $(OBJDIR)/%.o : %.cpp
	$(CC) $(CCFLAGS) -c $< -o $@
clean:
	rm -fr $(OBJS) $(EXE)

.PHONY:clean
