libutil.o: libutil.c libutil.h
	gcc -c libutil.c -o libutil.o
main.o: main.c libutil.h
	gcc -c main.c -o main.o
tool.o: tool.c libutil.h
	gcc -c tool.c -o tool.o
gen_data.o: gen_data.c
	gcc -c gen_data.c -o gen_data.o
data.bin: gen_data.o
	./gen_data > data.bin
app: main.o libutil.o data.bin
	gcc -o app main.o libutil.o
tool: tool.o libutil.o
	gcc -o tool tool.o libutil.o
