# 1. build 块：编译 foo.o 和 bar.o 并生成可执行文件 myapp
build: foo.o bar.o
	gcc -o myapp foo.o bar.o

# foo.o 规则
foo.o: src/foo.c include/foo.h
	gcc -Iinclude -c src/foo.c -o foo.o

# bar.o 规则
bar.o: src/bar.c include/bar.h
	gcc -Iinclude -c src/bar.c -o bar.o

# 2. test 块：依赖 build，执行单元测试
test: build
	./myapp --run-tests