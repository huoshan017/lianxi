SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp = .o)
	CC = g++
	INCLUDES = -I/×××
	LIBS = -L/×××
	CCFLAGS = -g -Wall -O0 -std=c++11

my_app : $(OBJS)
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS) -lboost_system

OBJS : %.cpp
	$(CC) -c $< $(CCFLAGS)
clean:
	rm *.o my_app
.PHONY:clean
