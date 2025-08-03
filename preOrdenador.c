// Responsável por pré ordenar e filtrar a quantidade de alunos a serem ordenados 
// Para compilar: gcc -o preOrdenador.exe preOrdenador.c src/utils.c -Isrc/include -std=c99
// Para executar: ./preOrdenador.exe

#include <stdio.h>    
#include <stdlib.h>  
#include <time.h>     
#include <string.h>   

#include "./src/include/common_types.h" 

// Função de comparação para qsort, para ordenar notas em ordem ascendente
int compararAscendente(const void *a, const void *b) {
    TipoAluno *alu_a = (TipoAluno *)a;
    TipoAluno *alu_b = (TipoAluno *)b;
    if (alu_a->nota < alu_b->nota) return -1;
    if (alu_a->nota > alu_b->nota) return 1;
    return 0;
}

// Função de comparação para qsort, para ordenar notas em ordem descendente
int compararDescendente(const void *a, const void *b) {
    TipoAluno *alu_a = (TipoAluno *)a;
    TipoAluno *alu_b = (TipoAluno *)b;
    if (alu_a->nota > alu_b->nota) return -1;
    if (alu_a->nota < alu_b->nota) return 1;
    return 0;
}

// Gera e escreve um arquivo de dados com a quantidade e ordem especificadas
void gerarArquivosBases(const char *filename, int situacao) {
    long total = 471705;

    FILE *file = fopen(filename, "w"); // Cria o arquivo em modo de escrita
    if (file == NULL) {
        perror("Erro ao criar o arquivo de dados");
        return;
    }

    // Aloca memória para armazenar todos os registros temporariamente
    TipoAluno *alunos = (TipoAluno *)malloc(total * sizeof(TipoAluno));
    if (alunos == NULL) {
        perror("Erro de alocacao de memoria para registros");
        fclose(file);
        return;
    }

    //le o arquivo base PROVAO
    FILE *inicial = fopen("data/PROVAO.TXT","r");

    //passa valores do provao.txt para memoria interna
    for(long i=0; i<total; i++){
        fscanf(inicial,"%8ld %5f",&alunos[i].insc,&alunos[i].nota);
        fseek(inicial,1,1);

        fgets(alunos[i].uf,3,inicial);
        fseek(inicial,1,1);

        fgets(alunos[i].cidade,51,inicial);
        fseek(inicial,1,1);

        fgets(alunos[i].curso,31,inicial);
        fseek(inicial,1,1);
    }

    // Aplica a ordenação conforme a situação especificada
    if (situacao == 1) {// Ordem ascendente
        qsort(alunos, total, sizeof(TipoAluno), compararAscendente);
    } else if (situacao == 2) { // Ordem descendente
        qsort(alunos, total, sizeof(TipoAluno), compararDescendente);
    } else {
        for (long i = total - 1; i > 0; i--) {
            long j = rand() % (i + 1); // Escolhe um índice aleatório
            TipoAluno temp = alunos[i]; // Troca os alunos
            alunos[i] = alunos[j];
            alunos[j] = temp;
        }
    }

    // Escreve todos os alunos (ordenados ou aleatórios) no arquivo
    for(long i=0; i<total; i++){
        fprintf(file, "%08ld %05.1f %2s %50s %30s\n", alunos[i].insc, alunos[i].nota, alunos[i].uf, alunos[i].cidade, alunos[i].curso);
    }

    free(alunos); 
    fclose(file);
    fclose(inicial);  
    printf("Arquivo '%s' gerado com %ld notas na situacao %d.\n", filename, total, situacao);
}

// Gera e escreve um arquivo de dados com a quantidade e ordem especificadas
void gerarArquivos(const char *filename, long qtd, int situacao) {
    FILE *file = fopen(filename, "w"); // Cria o arquivo em modo de escrita
    if (file == NULL) {
        perror("Erro ao criar o arquivo de dados");
        return;
    }

    // Aloca memória para armazenar todos os registros temporariamente
    TipoAluno *alunos = (TipoAluno *)malloc(qtd * sizeof(TipoAluno));
    if (alunos == NULL) {
        perror("Erro de alocacao de memoria para registros");
        fclose(file);
        return;
    }

    FILE *inicial;

    // Aplica a ordenação conforme a situação especificada
    if (situacao == 1) {// Ordem ascendente
        inicial = fopen("data/provao_471705_asc.txt","r");
    } else if (situacao == 2) { // Ordem descendente
        inicial = fopen("data/provao_471705_desc.txt","r");
    } else {
        inicial = fopen("data/provao_471705_rand.txt","r");
    }

    //passa valores do provao.txt para memoria interna
    for(long i=0; i<qtd; i++){
        fscanf(inicial,"%8ld %5f",&alunos[i].insc,&alunos[i].nota);
        fseek(inicial,1,1);

        fgets(alunos[i].uf,3,inicial);
        fseek(inicial,1,1);

        fgets(alunos[i].cidade,51,inicial);
        fseek(inicial,1,1);

        fgets(alunos[i].curso,31,inicial);
        fseek(inicial,1,1);
    }
    
    // Escreve todos os alunos (ordenados ou aleatórios) no arquivo
    for(long i=0; i<qtd; i++){
        fprintf(file, "%08ld %05.1f %2s %50s %30s\n", alunos[i].insc, alunos[i].nota, alunos[i].uf, alunos[i].cidade, alunos[i].curso);
    }

    free(alunos); 
    fclose(file);
    fclose(inicial);  
    printf("Arquivo '%s' gerado com %ld notas na situacao %d.\n", filename, qtd, situacao);
}

int main(int argc, char *argv[]) {
    printf("--- Geração de arquivos de pré ordenados para o TP02 ---\n");

    // Arrays predefinidos de quantidades de alunos e situações de ordem
    long quantidades[] = {100, 1000, 10000, 100000};
    int situacoes[] = {1,2,3}; // 1: ascendente, 2: descendente, 3: aleatoria
    char filename[256]; // Buffer para armazenar o nome do arquivo

    for (int s = 0; s < sizeof(situacoes) / sizeof(situacoes[0]); s++) {
        // Determina a string correspondente à situação de ordem
        const char* situacao_str;
        if (situacoes[s] == 1) situacao_str = "asc";
        else if (situacoes[s] == 2) situacao_str = "desc";
        else situacao_str = "rand";

        // Cria o nome do arquivo, direcionando-o para a pasta 'data/'
        sprintf(filename, "data/provao_471705_%s.txt",situacao_str);
        // Chama a função para gerar o arquivo 
        gerarArquivosBases(filename,situacoes[s]);
    }

    // Loops aninhados para gerar arquivos para todas as combinações de quantidade e situação
    for (int q = 0; q < sizeof(quantidades) / sizeof(quantidades[0]); q++) {
        for (int s = 0; s < sizeof(situacoes) / sizeof(situacoes[0]); s++) {
            // Determina a string correspondente à situação de ordem
            const char* situacao_str;
            if (situacoes[s] == 1) situacao_str = "asc";
            else if (situacoes[s] == 2) situacao_str = "desc";
            else situacao_str = "rand";

            // Cria o nome do arquivo, direcionando-o para a pasta 'data/'
            sprintf(filename, "data/provao_%ld_%s.txt", quantidades[q], situacao_str);
            // Chama a função para gerar o arquivo 
            gerarArquivos(filename, quantidades[q], situacoes[s]);
        }
    }

    printf("\nGeração de arquivos de pré ordenados concluída.\n");

    return 0;
}