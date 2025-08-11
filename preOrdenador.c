#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "./src/include/common_types.h"
#include "./src/include/utils.h" 

void trim_trailing_spaces(char *s)
{
    int i = strlen(s) - 1;
    while (i >= 0 && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r'))
    {
        s[i] = '\0';
        i--;
    }
}

// Função de comparação para qsort, para ordenar notas em ordem ascendente
int compararAscendente(const void *a, const void *b) {
    TipoRegistro *alu_a = (TipoRegistro *)a;
    TipoRegistro *alu_b = (TipoRegistro *)b;
    if (alu_a->nota < alu_b->nota) return -1;
    if (alu_a->nota > alu_b->nota) return 1;
    return 0;
}

// Função de comparação para qsort, para ordenar notas em ordem descendente
int compararDescendente(const void *a, const void *b) {
    TipoRegistro *alu_a = (TipoRegistro *)a;
    TipoRegistro *alu_b = (TipoRegistro *)b;
    if (alu_a->nota > alu_b->nota) return -1;
    if (alu_a->nota < alu_b->nota) return 1;
    return 0;
}

// Gera e escreve um arquivo de dados com a quantidade e ordem especificadas
void gerarArquivosBases(const char *filename, int situacao) {
    long total = 471705;

    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Erro ao criar o arquivo de dados");
        return;
    }

    TipoRegistro *alunos = (TipoRegistro *)malloc(total * sizeof(TipoRegistro));
    if (alunos == NULL) {
        perror("Erro de alocacao de memoria para registros");
        fclose(file);
        return;
    }

    FILE *inicial = fopen("data/PROVAO.TXT", "r");
    if (inicial == NULL) {
        perror("Erro ao abrir o arquivo PROVAO.TXT");
        free(alunos);
        fclose(file);
        return;
    }
    
    char linha[100];
    long i = 0;
    while(i < total && fgets(linha, sizeof(linha), inicial) != NULL) {
        if(strlen(linha) > 1) {
            char temp_inscricao[9];
            char temp_nota[6];
            
            strncpy(temp_inscricao, linha, 8);
            temp_inscricao[8] = '\0';
            trim_trailing_spaces(temp_inscricao);
            alunos[i].inscricao = atoll(temp_inscricao);
            
            strncpy(temp_nota, linha + 9, 5);
            temp_nota[5] = '\0';
            trim_trailing_spaces(temp_nota);
            alunos[i].nota = atof(temp_nota);

            strncpy(alunos[i].estado, linha + 15, 2);
            alunos[i].estado[2] = '\0';
            trim_trailing_spaces(alunos[i].estado);
            
            strncpy(alunos[i].cidade, linha + 18, 50);
            alunos[i].cidade[50] = '\0';
            trim_trailing_spaces(alunos[i].cidade);
            
            strncpy(alunos[i].curso, linha + 69, 30);
            alunos[i].curso[30] = '\0';
            trim_trailing_spaces(alunos[i].curso);
            
            i++;
        }
    }
    fclose(inicial);
    total = i;
    
    if (situacao == 1) {
        qsort(alunos, total, sizeof(TipoRegistro), compararAscendente);
    } else if (situacao == 2) {
        qsort(alunos, total, sizeof(TipoRegistro), compararDescendente);
    } else {
        srand(time(NULL));
        for (long j = total - 1; j > 0; j--) {
            long k = rand() % (j + 1);
            TipoRegistro temp = alunos[j];
            alunos[j] = alunos[k];
            alunos[k] = temp;
        }
    }

    fwrite(alunos, sizeof(TipoRegistro), total, file);
    
    free(alunos);
    fclose(file);
    printf("Arquivo '%s' gerado com %ld notas na situacao %d.\n", filename, total, situacao);
}

// Gera e escreve um arquivo de dados com a quantidade e ordem especificadas a partir dos arquivos binários base
void gerarArquivos(const char *filename, long qtd, int situacao) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Erro ao criar o arquivo de dados");
        return;
    }

    FILE *inicial;
    if (situacao == 1) {
        inicial = fopen("data/provao_471705_asc.bin", "rb");
    } else if (situacao == 2) {
        inicial = fopen("data/provao_471705_desc.bin", "rb");
    } else {
        inicial = fopen("data/provao_471705_rand.bin", "rb");
    }

    if (inicial == NULL) {
        perror("Erro ao abrir arquivo base para gerar subconjunto");
        fclose(file);
        return;
    }
    
    TipoRegistro *alunos = (TipoRegistro *)malloc(qtd * sizeof(TipoRegistro));
    if (alunos == NULL) {
        perror("Erro de alocacao de memoria para registros");
        fclose(file);
        fclose(inicial);
        return;
    }

    fread(alunos, sizeof(TipoRegistro), qtd, inicial);
    fwrite(alunos, sizeof(TipoRegistro), qtd, file);

    free(alunos);
    fclose(file);
    fclose(inicial);
    printf("Arquivo '%s' gerado com %ld notas na situacao %d.\n", filename, qtd, situacao);
}

int main(int argc, char *argv[]) {
    printf("--- Geração de arquivos de pré ordenados para o TP02 ---\n");

    long quantidades[] = {100, 1000, 10000, 100000};
    int situacoes[] = {1, 2, 3};
    char filename[256];

    for (int s = 0; s < sizeof(situacoes) / sizeof(situacoes[0]); s++) {
        const char* situacao_str;
        if (situacoes[s] == 1) situacao_str = "asc";
        else if (situacoes[s] == 2) situacao_str = "desc";
        else situacao_str = "rand";
        sprintf(filename, "data/provao_471705_%s.bin", situacao_str);
        gerarArquivosBases(filename, situacoes[s]);
    }
    
    for (int q = 0; q < sizeof(quantidades) / sizeof(quantidades[0]); q++) {
        for (int s = 0; s < sizeof(situacoes) / sizeof(situacoes[0]); s++) {
            const char* situacao_str;
            if (situacoes[s] == 1) situacao_str = "asc";
            else if (situacoes[s] == 2) situacao_str = "desc";
            else situacao_str = "rand";
            
            long qtd = quantidades[q];
            sprintf(filename, "data/provao_%ld_%s.bin", qtd, situacao_str);
            gerarArquivos(filename, qtd, situacoes[s]);

            if (qtd == 1000) {
                // A conversão para TXT agora é uma função separada, não precisa ser aqui
            }
        }
    }

    printf("\nGeração de arquivos de pré ordenados concluída.\n");

    return 0;
}