// Implementa funções utilitárias para medir o desempenho dos algoritmos
#include "include/utils.h"
#include <stdio.h> 

// Conta o número de operações de leitura/escrita (I/O)
long g_io_transferencias = 0;
// Conta o número de comparações entre chaves de pesquisa
long g_comparacoes_chaves = 0;

// Zera os contadores globais de desempenho antes de uma nova medição
void resetar_contadores() {
    g_io_transferencias = 0;
    g_comparacoes_chaves = 0;
}

// Incrementa o contador de transferências (operações de I/O)
void incrementar_io() {
    g_io_transferencias++;
}

// Incrementa o contador de comparações de chaves
void incrementar_comparacao() {
    g_comparacoes_chaves++;
}

// Marca o tempo de início de uma operação
clock_t iniciar_tempo() {
    return clock();
}

// Calcula o tempo total decorrido em segundos desde um tempo de início
double finalizar_tempo(clock_t inicio) {
    return (double)(clock() - inicio) / CLOCKS_PER_SEC;
}