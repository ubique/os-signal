CXX := g++
OPTIONS := -std=c++17 -Wall -pedantic

all: sigsegv-handler

sigsegv-handler: sigsegv-handler.o
	$(CXX) $(OPTIONS) sigsegv-handler.o -o sigsegv-handler

sigsegv-handler.o:
	$(CXX) $(OPTIONS) -c sigsegv-handler.cpp

clean:
	rm -rf *.o sigsegv-handler

