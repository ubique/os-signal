all: compile_project

compile_project: main.cpp Handler.cpp Utils.cpp
	g++ main.cpp Handler.cpp Utils.cpp -o signalTester

clean: 
	rm -rf ./signalTester
