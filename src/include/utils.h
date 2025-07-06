#ifndef UTILS_H
#define UTILS_H

#include <time.h> 
#include <stdbool.h> 

// Variáveis globais para contadores
extern long g_io_transferencias;
extern long g_comparacoes_chaves;

// Funções para gerenciar contadores
void resetar_contadores();
void incrementar_io();
void incrementar_comparacao();

// Funções para medir tempo
clock_t iniciar_tempo();
double finalizar_tempo(clock_t inicio);

#endif // UTILS_H