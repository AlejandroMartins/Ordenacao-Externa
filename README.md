#to compile main:
gcc -o pesquisa.exe main.c src/sequencial.c src/arvBin.c src/arvoreB.c src/arvoreEstrela.c src/utils.c -I./src/include -std=c99 -Wall -Wextra -g

#to execute main:
valgrind -s ./pesquisa.exe

#to compile preOrdenador:
gcc preOrdenador.c -o proOrdenador.exe -Wall

#to execute preOrdenador:
valgrind ./preOrdenador.exe
