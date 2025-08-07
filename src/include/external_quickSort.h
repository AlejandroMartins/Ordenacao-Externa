#ifndef EXTERNAL_QUICKSORT_H
#define EXTERNAL_QUICKSORT_H

#include "common_types.h"
#include <stdio.h>

// Função principal
void quicksort_externo(FILE *arq, long inicio, long fim);

// Função auxiliar para verificar integridade dos registros
void verificar_integridade(FILE *arq, long qtd);

#endif