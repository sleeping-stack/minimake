# 底层源文件 -> 目标对象文件
libutil.o: libutil.c libutil.h
	gcc -c libutil.c -o libutil.o

main.o: main.c libutil.h
	gcc -c main.c -o main.o

tool.o: tool.c libutil.h
	gcc -c tool.c -o tool.o

gen_data.o: gen_data.c
	gcc -c gen_data.c -o gen_data.o

# 生成数据文件（作为最终可执行依赖，确保有文件产出）
data.bin: gen_data.o
	./gen_data > data.bin

# 可执行程序1
app: main.o libutil.o data.bin
	gcc -o app main.o libutil.o

# 可执行程序2（共享 libutil.o）
tool: tool.o libutil.o
	gcc -o tool tool.o libutil.o