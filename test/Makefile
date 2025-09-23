libutil.o: libutil.c libutil.h
	gcc -c libutil.c -o libutil.o
main.o: main.c libutil.h
	gcc -c main.c -o main.o

app: main.o libutil.o
	gcc -o app main.o libutil.o
	echo "hahaha"