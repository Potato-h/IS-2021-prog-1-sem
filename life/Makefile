build:
	gcc -Wall -Wextra \
	life.c -o life \
	libbmp.c liblife.c hash_set.c

test: clean build
	./life --input start.bmp \
	--output out \
	--max_iter 10000

clean:
	rm -rf life out
