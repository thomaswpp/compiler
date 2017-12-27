flex tiny.l
bison -d tiny.y
gcc -c *.c
gcc -o main *.o -ly -lfl
./main file_teste 