all: build test

build:
	gcc -c uint1024.c

build-gcc:
	gcc -c uint1024.c

build-clang:
	clang -c uint1024.c

build-mingw:
	x86_64-w64-mingw32-gcc -c uint1024.c

test:
	gcc tests.c -o tests uint1024.o
	./tests

google-test:
	cd gtests && \
	cmake -S . -B build && \
	cmake --build build && \
	./build/tests

clean:
	rm -f tests uint1024.o
	rm -rf ./gtests/build	
