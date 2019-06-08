all: compile_utils compile_handler

compile_utils:
		g++ -std=c++11 -Wall -c -o utils.o utils.cpp;

compile_handler:
		g++ -std=c++11 -Wall -o main.o main.cpp
		
run:
		./main.o

clean:
		rm *.o