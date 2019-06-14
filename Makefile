GCC = g++
Main = main

all:  $(Main)

$(Main):
	$(GCC) -std=c++17 $(Main).cpp -o $(Main)

clean:
	rm $(Main)
