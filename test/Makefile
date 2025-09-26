CC = gcc
src = main.o libutil.o

libutil.o: libutil.c libutil.h
	${CC} -c libutil.c -o libutil.o
main.o: main.c libutil.h
	gcc -c main.c -o main.o

app: ${src}
	gcc -o app main.o libutil.o
	echo "hahaha"