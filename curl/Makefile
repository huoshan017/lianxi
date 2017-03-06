CC   = g++ -g  
COMPILE.C  = $(CC) -c   
VPATH   = ./  
CXXFLAGS  = -lcurl -std=c++11
MAKEEXE   = $(CC) $(LDFLAGS)  
OBJDIR = ./obj
ZSCOMMON_INC = $(PWD)/../../../xl_server/zscommon/include
ZSCOMMON_LIB = $(PWD)/../../../xl_server/zscommon/lib
  
OBJ = $(OBJDIR)/http_request.o \
	  $(OBJDIR)/http_request_processor.o \
	  $(OBJDIR)/http_request_mgr.o \
	  $(OBJDIR)/http_request_results.o \
	  $(OBJDIR)/http_postlog_mgr.o \
	  $(OBJDIR)/main.o 
EXE = main 
  
all: init  $(EXE)  
init:
	mkdir -p $(OBJDIR)

$(EXE): ${OBJ}  
	$(MAKEEXE) $(CXXFLAGS) $^ -o $@ $(ZSCOMMON_LIB)/libzscommon.a
$(OBJDIR)/%.o: %.cpp  
	$(COMPILE.C) $(CXXFLAGS) $< -o $@  -I$(ZSCOMMON_INC) 
clean:
	rm -f $(OBJDIR)/*.o $(EXE)  
