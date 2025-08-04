#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "src/include/common_types.h"
#include "src/include/utils.h"

int main(int argc, char *argv[])
{
    int metodo_escolhido;             // método de ordenação
    long quantidade_registros;        // número de registros no arquivo
    int situacao_ordem;               // situação de ordenação do arquivo
    bool exibir_chaves_debug = false; // flag para exibir todas as chaves do arquivo (descomentada para uso)

    char filename[256];
    FILE *arquivo_dados = NULL;

    // // Variáveis para medição de desempenho
    // clock_t inicio_tempo_geral;
    // double tempo_execucao_pesquisa;
    // double tempo_execucao_construcao_indice;

    // Verifica se o número mínimo de argumentos foi fornecido
    if (argc < 4)
    {
        fprintf(stderr, "Uso: %s <metodo> <quantidade> <situacao> [-P]\n", argv[0]);
        fprintf(stderr, "  <metodo>: 1=Intercalação Balanceada, 2=Seleção por Substituição, 3=QuickSort Externo\n");
        fprintf(stderr, "  <quantidade>: 100, 1000, 10000, 100000, 471705\n");
        fprintf(stderr, "  <situacao>: 1=Ascendente, 2=Descendente, 3=Aleatoria\n");
        fprintf(stderr, "  [-P]: Opcional, exibe chaves dos registros (debug)\n");
        return 1;
    }

    metodo_escolhido = atoi(argv[1]);
    quantidade_registros = atol(argv[2]);
    situacao_ordem = atoi(argv[3]);

    // Verifica se as chaves de pesquisa dos registros devem ser impressos
    if (argc >= 5 && strcmp(argv[4], "-P") == 0)
    {
        exibir_chaves_debug = true;
    }

    // Método inválido
    if (metodo_escolhido < 1 || metodo_escolhido > 3)
    {
        fprintf(stderr, "Erro: Metodo invalido. Escolha entre 1 e 3.\n");
        return 1;
    }

    // Quantidade de registros inválida
    if (quantidade_registros != 100 && quantidade_registros != 1000 &&
        quantidade_registros != 10000 && quantidade_registros != 100000 &&
        quantidade_registros != 471705)
    {
        fprintf(stderr, "Erro: Quantidade de registros invalida. Escolha 100, 1.000, 10.000, 100.000 ou 471.705.\n");
        return 1;
    }

    // Ordem inválida
    if (situacao_ordem < 1 || situacao_ordem > 3)
    {
        fprintf(stderr, "Erro: Situacao de ordem invalida. Escolha 1 (Ascendente), 2 (Descendente) ou 3 (Aleatoria).\n");
        return 1;
    }

    const char* situacao_str;
    if (situacao_ordem == 1) situacao_str = "asc";
    else if (situacao_ordem == 2) situacao_str = "desc";
    else situacao_str = "rand";

    sprintf(filename, "data/provao_%ld_%s.txt",quantidade_registros, situacao_str);

    // Abertura do arquivo para leitura
    arquivo_dados = fopen(filename, "rb");
    if (arquivo_dados == NULL)
    {
        fprintf(stderr, "Erro: Nao foi possivel abrir o arquivo de dados '%s'. Certifique-se de que ele foi gerado.\n", filename);
        return 1;
    }
    
    printf("--- Iniciando Ordenação ---\n");
    printf("Metodo: %d, Quantidade: %ld, Situacao: %d\n",
           metodo_escolhido, quantidade_registros, situacao_ordem);
    printf("Arquivo de dados: %s\n", filename);

    switch (metodo_escolhido)
    {
    case 1:
    { // 1: Intercalação balanceada - ordenação interna
        printf("Executando Intercalação Balanceada...\n");
        // Chamar função de intercalação balanceada aqui
        break;
    }

    case 2:
    { // 2: Intercalação balanceada - selec por subs
        printf("Executando Intercalação Balanceada com Seleção por Substituição...\n");
        // Chamar função de árvore binária externa aqui
        break;
    }

    case 3:
    { // 3: QuickSort Externo
        printf("Executando QuickSort Externo...\n");
        // Chamar função de QuickSort Externo aqui
        break;
    }
    }

    if (exibir_chaves_debug) {
        printf("\n--- Conteudo do Arquivo (todos os campos para debug) ---\n");
        fseek(arquivo_dados, 0, SEEK_SET);

        char linha[120]; // Buffer para a linha lida.
        TipoRegistro temp_reg;
        long long count = 0;


        while (fgets(linha, sizeof(linha), arquivo_dados) != NULL && count < quantidade_registros) {
            // Remove o caractere de nova linha ('\n') ou retorno de carro ('\r') se presente
            linha[strcspn(linha, "\n\r")] = 0;
         
            
            // --- INÍCIO DA EXTRAÇÃO DOS CAMPOS ---
            // Esses offsets e tamanhos são baseados na linha:
            // '00170838 034.8 MT CUIABA                                            ADMINISTRACAO                 '

            // 1. Inscrição (8 caracteres: colunas 1-8, índices 0-7)
            char temp_inscricao_str[9]; // 8 chars + '\0'
            strncpy(temp_inscricao_str, linha, 8);
            temp_inscricao_str[8] = '\0'; 
            temp_reg.inscricao = atoll(temp_inscricao_str);

            // 2. Nota (5 caracteres: colunas 10-14, índices 9-13)
            char temp_nota_str[6]; // 5 chars + '\0'
            strncpy(temp_nota_str, linha + 9, 5);
            temp_nota_str[5] = '\0';
            temp_reg.nota = atof(temp_nota_str);

            // 3. Estado (2 caracteres: colunas 16-17, índices 15-16)
            strncpy(temp_reg.estado, linha + 15, 2);
            temp_reg.estado[2] = '\0';

            // 4. Cidade (50 caracteres: colunas 19-68, índices 18-67)
            strncpy(temp_reg.cidade, linha + 18, 50);
            temp_reg.cidade[50] = '\0';
            trim_trailing_spaces(temp_reg.cidade); // Remove espaços excedentes

            // 5. Curso (30 caracteres: colunas 70-99, índices 69-98)
            strncpy(temp_reg.curso, linha + 69, 30);
            temp_reg.curso[30] = '\0';
            trim_trailing_spaces(temp_reg.curso); // Remove espaços excedentes
            
            // --- FIM DA EXTRAÇÃO DOS CAMPOS ---

            // Imprime todos os campos do registro para verificação
            printf("Inscricao: %lld, Nota: %.2f, Estado: '%s', Cidade: '%s', Curso: '%s'\n",
                   temp_reg.inscricao, temp_reg.nota, temp_reg.estado, temp_reg.cidade, temp_reg.curso);
            
            count++;
        }
        printf("\n--- Leitura de %lld registros concluída ---\n", count);
    }
    
    fclose(arquivo_dados);
    return 0;
}