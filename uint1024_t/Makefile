all: build test

build:
	gcc -c uint1024.c

test:
	gcc tests.c -o tests uint1024.o
	./tests

clean:
	rm -f tests uint1024.o	
