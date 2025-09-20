build: foo.o bar.o
	gcc -o myapp foo.o bar.o
foo.o: src/foo.c include/foo.h
	gcc -Iinclude -c src/foo.c -o foo.o
bar.o: src/bar.c include/bar.h
	gcc -Iinclude -c src/bar.c -o bar.o
test: build
	./myapp --run-tests
