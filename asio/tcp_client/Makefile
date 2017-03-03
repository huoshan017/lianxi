CC = g++
INCLUDES = -I/×××
LIBS = -L/×××
CCFLAGS = -g -Wall -O0 -std=c++11

COMMON_SRCS = $(wildcard ../libjmy/*.cpp)
COMMON_OBJS = $(COMMON_SRCS:.cpp = .o)
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp = .o)

my_app : $(OBJS) $(COMMON_OBJS)
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS) -lboost_system -lpthread

COMMON_OBJS : %.cpp
	$(CC) -c $< $(CCFLAGS)

OBJS : %.cpp
	$(CC) -c $< $(CCFLAGS)
clean:
	rm *.o my_app
.PHONY:clean
