all: minimake

minimake: build.o execute_cmd.o grammar_check.o graph.o main.o parse.o preprocess.o
	gcc -o minimake build.o execute_cmd.o grammar_check.o graph.o main.o parse.o preprocess.o

build.o: build.c build.h execute_cmd.h graph.h parse.h common.h
	gcc -c build.c -o build.o

execute_cmd.o: execute_cmd.c execute_cmd.h parse.h common.h
	gcc -c execute_cmd.c -o execute_cmd.o

grammar_check.o: grammar_check.c grammar_check.h common.h
	gcc -c grammar_check.c -o grammar_check.o

graph.o: graph.c graph.h parse.h common.h
	gcc -c graph.c -o graph.o

main.o: main.c build.h execute_cmd.h grammar_check.h graph.h parse.h preprocess.h common.h
	gcc -c main.c -o main.o

parse.o: parse.c parse.h grammar_check.h common.h
	gcc -c parse.c -o parse.o

preprocess.o: preprocess.c preprocess.h common.h
	gcc -c preprocess.c -o preprocess.o

clean:
	rm -f build.o execute_cmd.o grammar_check.o graph.o main.o parse.o preprocess.o