all: callback_test
.PHONY: all

callback_test:
	gcc -std=c99 -Wall -Werror -g -I../../inc -I../../../ibase/inc -I../../../tasksch/inc ../src/callback_test.c -c
	gcc -std=c99 -Wall -Werror -g -D UNIT_TEST=1 -I../inc -I../../inc -I../../../ibase/inc -I../../../tasksch/inc  ../../src/callback.c -c
	gcc callback.o callback_test.o -o ../bin/callback_test

clean:
	rm -f ../bin/callback_test callback.o callback_test.o

