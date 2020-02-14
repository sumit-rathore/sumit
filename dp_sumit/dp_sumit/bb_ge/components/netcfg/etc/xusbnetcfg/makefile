all: xusbnetcfglib.a xusbnetcfg${EXE} xusbnet_test_target${EXE}

CFLAGS=-g -O0 -I. -Wall

win:
	${MAKE} LDFLAGS=-lws2_32 EXE=.exe

xusbnetcfglib.a: xusbnetcfglib.c xusbnetcfg.h
	gcc ${CFLAGS} -c $< -o xusbnetcfglib.o
	ar rcs $@ xusbnetcfglib.o

xusbnetcfg${EXE}: xusbnetcfg.c xusbnetcfglib.a xusbnetcfg.h
	gcc ${CFLAGS} $< xusbnetcfglib.a ${LDFLAGS} -o $@

xusbnet_test_target${EXE}: xusbnet_test_target.c xusbnetcfg.h
	gcc ${CFLAGS} $< ${LDFLAGS} -o $@

docs:
	sed "s/<PROJECT_NUMBER>/`git show --format=%H | head -n 1`/" .doxygen_cfg.template > .doxygen_cfg
	doxygen .doxygen_cfg

clean:
	rm -f xusbnetcfglib.a xusbnetcfglib.o xusbnetcfg xusbnet_test_target xusbnetcfg.exe xusbnet_test_target.exe xusbnetcfg${EXE} xusbnet_test_target${EXE}
	rm -rf docs/
	rm -f .doxygen_cfg
