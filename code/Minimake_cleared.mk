app: main.c utils.c
    gcc -o app main.c utils.c
    git apply  xxx.patch
    echo "success build app"
