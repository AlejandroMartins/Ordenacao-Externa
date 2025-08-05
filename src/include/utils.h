#ifndef UTILS_H
#define UTILS_H

#include "common_types.h"
#include <time.h>
#include <string.h> // Inclua string.h aqui, pois é onde trim_trailing_spaces é declarado

extern long g_io_transferencias;
extern long g_comparacoes_chaves;

void resetar_contadores();
void incrementar_io();
void incrementar_comparacao();
clock_t iniciar_tempo();
double finalizar_tempo(clock_t inicio);
int comparadorNotas(const void *a, const void *b);
void trim_trailing_spaces(char *str);
void converterBinarioParaTexto(const char *arquivo_binario, const char *arquivo_texto, long quantidade_registros);

#endif