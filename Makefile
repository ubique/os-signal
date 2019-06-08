.PHONY: all clean

all: main.c
	cc main.c -o sigsegv_test

clean:
	-rm -rf sigsegv_test

