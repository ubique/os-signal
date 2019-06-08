signal: signal.cpp
	c++ -std=c++14 $^ -o $@

clean:
	rm -rf signal
