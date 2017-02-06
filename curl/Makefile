CC   = g++ -g  
COMPILE.C  = $(CC) -c   
VPATH   = ./  
CXXFLAGS  =  -lcurl -std=c++11  
MAKEEXE   = $(CC) $(LDFLAGS)  
OBJDIR = ./obj
  
OBJ = $(OBJDIR)/http_request.o \
	  $(OBJDIR)/http_request_pool.o \
	  $(OBJDIR)/http_request_list.o \
	  $(OBJDIR)/http_request_processor.o \
	  $(OBJDIR)/http_request_mgr.o \
	  $(OBJDIR)/http_request_results.o \
	  $(OBJDIR)/main.o 
EXE = main 
  
all: init  $(EXE)  
init:
	mkdir -p $(OBJDIR)

$(EXE): ${OBJ}  
	$(MAKEEXE) $(CXXFLAGS) $^ -o $@  
$(OBJDIR)/%.o: %.cpp  
	$(COMPILE.C) $(CXXFLAGS) $< -o $@  
clean:
	rm -f $(OBJDIR)/*.o $(EXE)  
