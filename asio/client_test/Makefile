CC = g++
INCLUDES = -I/××× -I../net_tcp
LIBS = -L/×××
CCFLAGS = -g -Wall -O0 -std=c++11
OBJDIR = ./obj
EXE = client_test

COMMON_SRCS = $(wildcard ../libjmy/*.cpp)
COMMON_OBJS = $(patsubst ../libjmy/%.cpp, ../libjmy/%.o, $(COMMON_SRCS))
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SRCS))

all: init $(EXE)
init:
	mkdir -p $(OBJDIR)

$(EXE): $(OBJS) $(COMMON_OBJS)
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS) -lboost_system -lpthread

$(COMMON_OBJS) : ../libjmy/%.o: ../libjmy/%.cpp
	$(CC) -c $< $(CCFLAGS) -o $@

$(OBJS) : $(OBJDIR)/%.o : %.cpp
	$(CC) -c $< $(CCFLAGS) -o $@

clean:
	rm -fr $(COMMON_OBJS) $(OBJS) $(EXE)

.PHONY:clean
