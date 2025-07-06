#to compile main:
gcc -o pesquisa.exe main.c src/sequencial.c src/arvBin.c src/arvoreB.c src/arvoreEstrela.c src/utils.c -I./src/include -std=c99 -Wall -Wextra -g

#to execute main:
valgrind -s ./pesquisa.exe

#to compile dataGenerator:
gcc -o dataGenerator.exe dataGenerator.c src/utils.c -I./src/include -std=c99 -Wall -Wextra -g

#to execute dataGenerator:
valgrind -s ./dataGenerator.exe
