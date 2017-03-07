CC = clang++
JMYDIR = ../libjmy
INCLUDES = -I/×××
LIBS = -L/××× 
CCFLAGS = -g -Wall -O0 -std=c++11
EXE = tcp_server
JMYOBJDIR = $(JMYDIR)/obj
OBJDIR = ./obj

COMMON_SRCS = $(wildcard $(JMYDIR)/*.cpp)
COMMON_OBJS = $(patsubst $(JMYDIR)/%.cpp, $(JMYOBJDIR)/%.o, $(COMMON_SRCS))
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SRCS))

all: init $(EXE)
init:
	mkdir -p $(OBJDIR)

$(EXE): $(OBJS) $(COMMON_OBJS)
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS) -lboost_system -lpthread $(JMYDIR)/thirdparty/lib/zlog/libzlog.a

$(COMMON_OBJS) : $(JMYOBJDIR)/%.o : $(JMYDIR)/%.cpp
	$(CC) -c $< $(CCFLAGS) -o $@

$(OBJS) : $(OBJDIR)/%.o : %.cpp
	$(CC) -c $< $(CCFLAGS) -o $@
clean:
	rm $(COMMON_OBJS) $(OBJS)
.PHONY:clean
