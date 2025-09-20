app: main.c utils.c
	gcc -o app main.c utils.c
	git apply  xxx.patch
app: main.c utils.c
	echo "success build app"
