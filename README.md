#to compile main:
gcc main.c src/balanceada.c src/external_quickSort.c src/utils.c -o main.exe -Wall -g

#to execute main:
valgrind ./main.exe

#to compile preOrdenador:
gcc preOrdenador.c -o preOrdenador.exe -Wall -g

#to execute preOrdenador:
valgrind ./preOrdenador.exe
