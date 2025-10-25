minimake: build.o clioptions_parse.O
	gcc -o minimake build.o clioptions_parse.O

build.o: build.c
	gcc -o build.o build.c