CC = gcc
CC = clang
CFLAGS = -O1
CFLAGS = -O2
BUILD_CMD = ${CC} $(CFLAGS)
src = main.o libutil.o
tag = app
all: $(tag)
libutil.o: libutil.c libutil.h
	${CC} ${CFLAGS} -c libutil.c -o libutil.o
main.o: main.c libutil.h
	$(CC) ${CFLAGS} -c main.c -o main.o
$(tag): ${src}
	$(BUILD_CMD) -o $(tag) $(src)
	echo "hahaha"
