CC = clang++
JMYDIR = ../libjmy
INCLUDES = -I/×××
LIBS = -L/××× -L$(JMYDIR)/thirdparty/lib/zlog
CCFLAGS = -g -Wall -O0 -std=c++11
OBJDIR = ./obj
JMYOBJDIR = $(JMYDIR)/obj
EXE = multi_clients

COMMON_SRCS = $(wildcard $(JMYDIR)/*.cpp)
COMMON_OBJS = $(patsubst $(JMYDIR)/%.cpp, $(JMYOBJDIR)/%.o, $(COMMON_SRCS))
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SRCS))

all: init $(EXE)  
init:
	mkdir -p $(OBJDIR) $(JMYOBJDIR)

$(EXE): $(OBJS) $(COMMON_OBJS)
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS) -lboost_system -lboost_thread -lpthread $(JMYDIR)/thirdparty/lib/zlog/libzlog.a

$(COMMON_OBJS) : $(JMYOBJDIR)/%.o: $(JMYDIR)/%.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

$(OBJS) : $(OBJDIR)/%.o : %.cpp
	$(CC) $(CCFLAGS) -c $< -o $@
clean:
	rm -fr $(COMMON_OBJS) $(OBJS) $(EXE)

.PHONY:clean
