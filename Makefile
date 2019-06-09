all: hw
	
hw: main.cpp Handler.cpp
	g++ main.cpp Handler.cpp -o signals

clean:
	rm -rf *.o signals

