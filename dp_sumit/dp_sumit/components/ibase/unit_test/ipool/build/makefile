all:
	gcc -std=gnu99 -m32 -Wall -Os -I ../../../inc ../src/ipool_test.c -c
	gcc -std=gnu99 -m32 -Wall -Os -I ../../../inc ../../../src/ipool.c -c
	gcc -m32 ipool.o ipool_test.o -o ../bin/ipool

clean:
	rm -f ../bin/ipool ipool.o ipool_test.o

