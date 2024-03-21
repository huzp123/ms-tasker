CXX=g++
CXXFLAGS=-c -Wall -std=c++11 -pthread
LDFLAGS= -lpthread
 
all: ms-tasker

ms-tasker: ms-tasker.o
	$(CXX) -o $@ $^ $(LDFLAGS)
	@rm -f *.o
 
ms-tasker.o: ms_tasker.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f *.o ms-tasker