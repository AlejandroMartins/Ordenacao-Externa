// Implementa funções utilitárias para medir o desempenho dos algoritmos
#include <stdio.h> 
#include <string.h>
#include "./include/utils.h"
#include "./include/common_types.h"

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

int comparadorNotas(const void *a, const void *b) {
    TipoRegistro *reg1 = (TipoRegistro *)a;
    TipoRegistro *reg2 = (TipoRegistro *)b;
    
    if (reg1->nota < reg2->nota) {
        return -1;
    }
    if (reg1->nota > reg2->nota) {
        return 1;
    }
    return 0;
}

void trim_trailing_spaces(char *s)
{
    int i = strlen(s) - 1;
    while (i >= 0 && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r'))
    {
        s[i] = '\0';
        i--;
    }
}


void converterBinarioParaTexto(const char *arquivo_binario, const char *arquivo_texto, long quantidade_registros)
{
    FILE *bin_file = fopen(arquivo_binario, "rb");
    if (bin_file == NULL)
    {
        fprintf(stderr, "Erro ao abrir o arquivo binario '%s'.\n", arquivo_binario);
        return;
    }

    FILE *txt_file = fopen(arquivo_texto, "w");
    if (txt_file == NULL)
    {
        fprintf(stderr, "Erro ao criar o arquivo de texto '%s'.\n", arquivo_texto);
        fclose(bin_file);
        return;
    }

    TipoRegistro reg;
    long count = 0;

    printf("Convertendo arquivo binario para texto...\n");
    while (fread(&reg, sizeof(TipoRegistro), 1, bin_file) == 1 && count < quantidade_registros)
    {
        fprintf(txt_file, "%08lld %05.1f %s %s %s\n",
                reg.inscricao, reg.nota, reg.estado, reg.cidade, reg.curso);
        count++;
    }

    printf("Conversao concluida. %ld registros escritos em '%s'.\n", count, arquivo_texto);

    fclose(bin_file);
    fclose(txt_file);
}