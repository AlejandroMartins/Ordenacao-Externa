#include "include/external_quickSort.h"
#include "include/utils.h"
#include <stdlib.h>
#include <limits.h>
#include <string.h>

// Tamanho máximo da memória interna (20 registros) -> pivô
#define TAM_AREA 20

// Estrutura para a área de pivô (heap mínimo)
typedef struct {
    TipoRegistro registros[TAM_AREA];
    int num_registros;
} TipoArea;

// --- Funções Auxiliares ---

// Insere um registro na área de forma ordenada (usando inserção direta)
static void inserir_area(TipoArea *area, TipoRegistro reg) {
    int i = area->num_registros;
    while (i > 0 && area->registros[i-1].nota > reg.nota) {
        area->registros[i] = area->registros[i-1];
        i--;
        incrementar_comparacao();
    }
    area->registros[i] = reg;
    area->num_registros++;
}

// Remove o menor registro da área
static void remover_min(TipoArea *area, TipoRegistro *reg) {
    *reg = area->registros[0];
    for (int i = 0; i < area->num_registros - 1; i++) {
        area->registros[i] = area->registros[i+1];
    }
    area->num_registros--;
}

// Remove o maior registro da área
static void remover_max(TipoArea *area, TipoRegistro *reg) {
    *reg = area->registros[area->num_registros - 1];
    area->num_registros--;
}

// --- Função de Partição com Área de Pivô ---
static void particao(FILE *arq, long inicio, long fim, long *i, long *j) {
    TipoArea area;
    area.num_registros = 0;

    long Li = inicio;   // Ponteiro de leitura inicial
    long Ls = fim;      // Ponteiro de leitura final
    long Ei = inicio;   // Ponteiro de escrita inicial
    long Es = fim;      // Ponteiro de escrita final
    float Linf = -1.0;  // Limite inferior (notas mínimas são 0.0)
    float Lsup = 101.0; // Limite superior (notas máximas são 100.0)
    short onde_ler = 0; // 0 = início, 1 = fim

    *i = inicio - 1;
    *j = fim + 1;

    while (Li <= Ls) {
        if (area.num_registros < TAM_AREA) {
            TipoRegistro reg;
            if (onde_ler == 0) {
                fseek(arq, Li * sizeof(TipoRegistro), SEEK_SET);
                fread(&reg, sizeof(TipoRegistro), 1, arq);
                Li++;
                onde_ler = 1;
            } else {
                fseek(arq, Ls * sizeof(TipoRegistro), SEEK_SET);
                fread(&reg, sizeof(TipoRegistro), 1, arq);
                Ls--;
                onde_ler = 0;
            }
            incrementar_io();

            if (reg.nota < Linf) {
                fseek(arq, Ei * sizeof(TipoRegistro), SEEK_SET);
                fwrite(&reg, sizeof(TipoRegistro), 1, arq);
                Ei++;
                incrementar_io();
            } else if (reg.nota > Lsup) {
                fseek(arq, Es * sizeof(TipoRegistro), SEEK_SET);
                fwrite(&reg, sizeof(TipoRegistro), 1, arq);
                Es--;
                incrementar_io();
            } else {
                inserir_area(&area, reg);
            }
            continue;
        }

        // Área cheia: balanceia subvetores
        TipoRegistro reg_removido;
        long T1 = Ei - inicio;
        long T2 = fim - Es;

        if (T1 < T2) {
            remover_min(&area, &reg_removido);
            fseek(arq, Ei * sizeof(TipoRegistro), SEEK_SET);
            fwrite(&reg_removido, sizeof(TipoRegistro), 1, arq);
            Ei++;
            Linf = reg_removido.nota;
        } else {
            remover_max(&area, &reg_removido);
            fseek(arq, Es * sizeof(TipoRegistro), SEEK_SET);
            fwrite(&reg_removido, sizeof(TipoRegistro), 1, arq);
            Es--;
            Lsup = reg_removido.nota;
        }
        incrementar_io();
    }

    // Esvazia a área no arquivo
    while (area.num_registros > 0) {
        TipoRegistro reg_removido;
        remover_min(&area, &reg_removido);
        fseek(arq, Ei * sizeof(TipoRegistro), SEEK_SET);
        fwrite(&reg_removido, sizeof(TipoRegistro), 1, arq);
        Ei++;
        incrementar_io();
    }

    *i = Ei - 1;
    *j = Es + 1;
}

// --- Quicksort Externo Principal ---
void quicksort_externo(FILE *arq, long inicio, long fim) {
    if (fim - inicio < TAM_AREA) {
        // Caso base: ordenação interna (insertion sort)
        long num_registros = fim - inicio + 1;
        TipoRegistro *memoria = malloc(num_registros * sizeof(TipoRegistro));
        
        fseek(arq, inicio * sizeof(TipoRegistro), SEEK_SET);
        fread(memoria, sizeof(TipoRegistro), num_registros, arq);
        incrementar_io();

        for (long i = 1; i < num_registros; i++) {
            TipoRegistro chave = memoria[i];
            long j = i - 1;
            while (j >= 0 && memoria[j].nota > chave.nota) {
                incrementar_comparacao();
                memoria[j + 1] = memoria[j];
                j--;
            }
            incrementar_comparacao();
            memoria[j + 1] = chave;
        }

        fseek(arq, inicio * sizeof(TipoRegistro), SEEK_SET);
        fwrite(memoria, sizeof(TipoRegistro), num_registros, arq);
        incrementar_io();
        
        free(memoria);
    } else {
        long i, j;
        particao(arq, inicio, fim, &i, &j);

        // Ordena primeiro o menor subarquivo
        if ((i - inicio) < (fim - j)) {
            quicksort_externo(arq, inicio, i);
            quicksort_externo(arq, j, fim);
        } else {
            quicksort_externo(arq, j, fim);
            quicksort_externo(arq, inicio, i);
        }
    }
}

// --- Função de Verificação de Integridade ---
void verificar_integridade(FILE *arq, long qtd) {
    rewind(arq);
    TipoRegistro reg;

    for (long i = 0; i < qtd; i++) {
        fread(&reg, sizeof(TipoRegistro), 1, arq);
        printf("inscricao: %08lld\tnota: %.2f\testado: %s\tcurso: %s\n", reg.inscricao, reg.nota, reg.estado, reg.curso);
    }
}