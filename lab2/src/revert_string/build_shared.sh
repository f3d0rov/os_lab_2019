gcc -c -fPIC revert_string.c -o shared/revert_string.o
gcc -shared shared/revert_string.o -o shared/librevert_string.so