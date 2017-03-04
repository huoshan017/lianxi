CC   = g++ -g  
COMPILE.C  = $(CC) -c   
VPATH   = ./  
CXXFLAGS  =  -lcurl -std=c++11  
MAKEEXE   = $(CC) $(LDFLAGS)  
OBJDIR = ./obj
  
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SRCS))
	#$(OBJDIR)/http_request.o \
	  #$(OBJDIR)/http_request_processor.o \
	  #$(OBJDIR)/http_request_mgr.o \
	  #$(OBJDIR)/http_request_results.o \
	  #$(OBJDIR)/main.o 

EXE = main 
all: init  $(EXE)  
init:
	mkdir -p $(OBJDIR)

$(EXE): $(OBJS)
	$(MAKEEXE) $(CXXFLAGS) $^ -o $@

$(OBJS): $(OBJDIR)/%.o: %.cpp
	$(COMPILE.C) $(CXXFLAGS) $< -o $@  

clean:
	rm -f $(OBJS) $(EXE)  
