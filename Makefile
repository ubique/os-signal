CXX=g++
NAME=sigsegv
OUTPUT=output

.PHONY: all clean

all:
	@rm -rf ${OUTPUT}
	@mkdir ${OUTPUT}
	@${CXX} main.cpp handler/*.cpp table/*.cpp -o ${OUTPUT}/${NAME}

clean:
	@rm -rf ${OUTPUT}
