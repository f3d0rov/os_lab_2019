gcc -c main.c -o obj/main.o
gcc obj/main.o -Lshared -lrevert_string -o shared/revert_string