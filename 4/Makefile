build: test1 test2

test2: test2.c funcs.o lib
	gcc test2.c src/funcs.o -ldl -o test2

test1: test1.c funcs.o librealization1.so
	gcc test1.c src/funcs.o -Llib -lrealization1 -o test1

funcs.o: src/funcs.c include/funcs.h
	gcc -c src/funcs.c -o src/funcs.o

lib: librealization1.so librealization2.so

librealization1.so : lib/realization1.c
	gcc -shared -fPIC lib/realization1.c -o lib/librealization1.so
librealization2.so : lib/realization2.c
	gcc -shared -fPIC lib/realization2.c -o lib/librealization2.so