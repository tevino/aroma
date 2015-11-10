
all: ../bin/aroma

../bin:
	mkdir -p bin

../bin/aroma: ../bin *.c
	cc -g -std=c99 -Wall *.c -ledit -o bin/aroma
