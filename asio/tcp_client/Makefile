CC = g++
INCLUDES = -I/×××
LIBS = -L/×××
CCFLAGS = -g -Wall -O0 -std=c++11
OBJDIR = ./obj
EXE = tcp_client

COMMON_SRCS = $(wildcard ../libjmy/*.cpp)
COMMON_OBJS = $(patsubst ../libjmy/%.cpp, ../libjmy/%.o, $(COMMON_SRCS))
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

all: $(EXE)  
init:
	echo $(COMMON_OBJS) $(OBJS)
	mkdir -p $(OBJDIR)

$(EXE): $(OBJS) $(COMMON_OBJS)
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS) -lboost_system -lpthread

$(COMMON_OBJS) : ../libjmy/%.o: ../libjmy/%.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

$(OBJS) : %.o : %.cpp
	$(CC) $(CCFLAGS) -c $< -o $@
clean:
	rm -fr $(COMMON_OBJS) $(OBJS) $(EXE)

.PHONY:clean
