all : fifo_test
.PHONY : all

fifo_test :
	gcc -Wall -O3 -std=gnu99 -I ../../../inc ../src/fifo_test.c -o fifo
	mv fifo ../bin/fifo

clean: 
	rm -f ../bin/fifo
