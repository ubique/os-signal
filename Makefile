all: compile_project

compile_project: main.cpp Handler.cpp
	g++ main.cpp Handler.cpp -o signalTester

clean: 
	rm -rf ./signalTester
