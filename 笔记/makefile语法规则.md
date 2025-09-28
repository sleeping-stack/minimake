# makefile语法规则

## 基本语法规则

~~~
target: dependencies
	command
~~~

- 目标行必须顶格，命令行必须以tab开头

## 伪目标

~~~
.PHONY: clean all

all: app1 app2
	echo "Build all applications"
	
clean:
	rm *.o app
~~~

- make会默认执行第一条规则
- 如果目录下有`clean`文件则需声明clean为伪目标

## 变量替换

~~~
CFLAGS = -Wall -g -O2
$(CFLAGS)

target = 
sources = 
objects = 

$(target)
~~~

## 自动变量

~~~
$@   替换目标文件
@cmd 不会将命令打印到终端
$<   第一个依赖文件
~~~

## 通配符

~~~
%.o
%.c
~~~

