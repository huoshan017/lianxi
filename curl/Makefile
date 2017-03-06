CC   = g++ -g  
COMPILE.C  = $(CC) -c   
VPATH   = ./  
CXXFLAGS  = -lcurl -std=c++11
MAKEEXE   = $(CC) $(LDFLAGS)  
OBJDIR = ./obj
ZSCOMMON_INC = $(PWD)/../../../xl_server/zscommon/include
ZSCOMMON_LIB = $(PWD)/../../../xl_server/zscommon/lib
  
<<<<<<< HEAD
OBJ = $(OBJDIR)/http_request.o \
	  $(OBJDIR)/http_request_processor.o \
	  $(OBJDIR)/http_request_mgr.o \
	  $(OBJDIR)/http_request_results.o \
	  $(OBJDIR)/http_postlog_mgr.o \
	  $(OBJDIR)/main.o 
=======
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SRCS))
	#$(OBJDIR)/http_request.o \
	  #$(OBJDIR)/http_request_processor.o \
	  #$(OBJDIR)/http_request_mgr.o \
	  #$(OBJDIR)/http_request_results.o \
	  #$(OBJDIR)/main.o 

>>>>>>> dc29c3f7d9212c2dbac9cf996cdfbe491dcad038
EXE = main 
all: init  $(EXE)  
init:
	mkdir -p $(OBJDIR)

<<<<<<< HEAD
$(EXE): ${OBJ}  
	$(MAKEEXE) $(CXXFLAGS) $^ -o $@ $(ZSCOMMON_LIB)/libzscommon.a
$(OBJDIR)/%.o: %.cpp  
	$(COMPILE.C) $(CXXFLAGS) $< -o $@  -I$(ZSCOMMON_INC) 
=======
$(EXE): $(OBJS)
	$(MAKEEXE) $(CXXFLAGS) $^ -o $@

$(OBJS): $(OBJDIR)/%.o: %.cpp
	$(COMPILE.C) $(CXXFLAGS) $< -o $@  

>>>>>>> dc29c3f7d9212c2dbac9cf996cdfbe491dcad038
clean:
	rm -f $(OBJS) $(EXE)  
