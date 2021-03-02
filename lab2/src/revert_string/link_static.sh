gcc -c main.c -o obj/main.o
gcc obj/main.o -Lstatic -lrevert_string -o static/revert_string