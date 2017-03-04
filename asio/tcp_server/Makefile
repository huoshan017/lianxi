CC = g++
INCLUDES = -I/×××
LIBS = -L/×××
CCFLAGS = -g -Wall -O0 -std=c++11
EXE = tcp_server
OBJDIR = ./obj

COMMON_SRCS = $(wildcard ../libjmy/*.cpp)
COMMON_OBJS = $(patsubst ../libjmy/%.cpp, ../libjmy/%.o, $(COMMON_SRCS))
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

all: init $(EXE)

init:
	mkdir -p $(OBJDIR)
$(EXE): $(OBJS) $(COMMON_OBJS)
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS) -lboost_system -lpthread

$(COMMON_OBJS) : ../libjmy/%.o : ../libjmy/%.cpp
	$(CC) -c $< $(CCFLAGS) -o $@

$(OBJS) : %.o : %.cpp
	$(CC) -c $< $(CCFLAGS) -o $@
clean:
	rm $(COMMON_OBJS) $(OBJS)
.PHONY:clean
