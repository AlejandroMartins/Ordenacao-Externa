#ifndef BALANCEADA_H
#define BALANCEADA_H

#include "./common_types.h"
#include "./utils.h" // Corrigido para "utils.h", garantindo que ele está no mesmo diretório.
#include <stdio.h>

// Funções auxiliares de ordenação
short todosBlocosEsgotados(short ativas[], int n);
int restaUmaFitaPreenchida(int nBlocos[], int n, int inicio);
void intercalacao_balanceada(FILE **fitas, long n_registros, int n_blocos_iniciais[]);
void heapify(TipoRegistro arr[], int n, int i, short congelados[]);

// Métodos de ordenação externos do trabalho
void metodo_intercalacao_ordenacao(const char *entrada, long n_registros);
void metodo_intercalacao_selecao(const char *entrada, long n_registros);

#endif