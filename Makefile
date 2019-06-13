CXX=g++
NAME=sigsegv
OUTPUT=output

.PHONY: all clean

all:
	@rm -rf ${OUTPUT}
	@mkdir ${OUTPUT}
	@${CXX} *.cpp handler/*.cpp -o ${OUTPUT}/${NAME}

clean:
	@rm -rf ${OUTPUT}
