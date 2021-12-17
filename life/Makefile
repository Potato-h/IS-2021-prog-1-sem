build:
	gcc -Wall -Wextra \
	src/life.c -o life \
	lib/libbmp.c lib/liblife.c lib/hash_set.c

test: clean build
	./life --input start.bmp \
	--output out \
	--max_iter 10000

clean:
	rm -rf life out
