all: signal

run: signal
	./signal

signal:
	g++ -std=c++11 signal.cpp -o signal

clean:
	rm signal