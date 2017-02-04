CC   = g++ -g  
COMPILE.C  = $(CC) -c   
VPATH   = ./  
CXXFLAGS  =  -lcurl -lboost_system -std=c++11  
MAKEEXE   = $(CC) $(LDFLAGS)  
  
OBJ   = http_request.o http_request_pool.o http_request_list.o http_request_processor.o http_request_mgr.o main.o 
EXE   = main 
    
all:   $(EXE)  
	  
$(EXE):   ${OBJ}  
	    $(MAKEEXE) $(CXXFLAGS) $^ -o $@  
%.o:   %.cpp  
	    $(COMPILE.C) $(CXXFLAGS) $< -o $@  
clean:  
	    rm -f *.o $(EXE)  
