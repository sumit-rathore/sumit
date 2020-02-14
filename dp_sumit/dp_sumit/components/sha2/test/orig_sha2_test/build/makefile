all: ../bin/sha2test
.PHONY: all

../bin/sha2test: ../../../src/sha2.c ../src/sha2test.c
	mkdir -p ../bin
	gcc -Wall -Os -I../../../inc -I../../../../ibase/inc $^ -o $@

clean:
	-rm ../bin/sha2test
	-rmdir ../bin

