#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "./src/include/common_types.h"
#include "./src/include/utils.h"
#include "./src/include/balanceada.h"
#include "./src/include/external_quickSort.h"

int main(int argc, char *argv[])
{
    int metodo_escolhido;             // método de ordenação
    long quantidade_registros;        // número de registros no arquivo
    int situacao_ordem;               // situação de ordenação do arquivo
    bool exibir_chaves_debug = false; // flag para ativar modo debug (exibir registros)
    char filename[256];
    FILE *arquivo_dados = NULL;

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

    // Converte argumentos para variáveis internas
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

    // Define string para nome do arquivo conforme a situação
    const char *situacao_str;
    if (situacao_ordem == 1)
        situacao_str = "asc";
    else if (situacao_ordem == 2)
        situacao_str = "desc";
    else
        situacao_str = "rand";

    // Monta o nome do arquivo de entrada
    sprintf(filename, "data/provao_%ld_%s.bin", quantidade_registros, situacao_str);

    // Abertura do arquivo para leitura
    arquivo_dados = fopen(filename, "rb");
    // Caso não encontre arquivo
    if (arquivo_dados == NULL)
    {
        fprintf(stderr, "Erro: Nao foi possivel abrir o arquivo de dados '%s'. Certifique-se de que ele foi gerado.\n", filename);
        return 1;
    }

    // Informações iniciais sobre o experimento
    printf("--- Iniciando Ordenação ---\n");
    printf("Metodo: %d, Quantidade: %ld, Situacao: %d\n",
           metodo_escolhido, quantidade_registros, situacao_ordem);
    printf("Arquivo de dados: %s\n", filename);

    resetar_contadores(); // Reseta contadores globais de comparações e transferências
    clock_t inicio_tempo = iniciar_tempo(); // Marca o início do tempo para medir duração

    // Seleciona e executa o método escolhido
    switch (metodo_escolhido)
    {
    case 1:
    { // 1: Intercalação balanceada - ordenação interna
        printf("Executando Intercalação Balanceada com Ordenação Interna...\n");
        metodo_intercalacao_ordenacao(filename, quantidade_registros);
        fclose(arquivo_dados);
        break;
    }

    case 2:
    { // 2: Intercalação balanceada - selec por substituição
        printf("Executando Intercalação Balanceada com Seleção por Substituição...\n");
        metodo_intercalacao_selecao(filename, quantidade_registros);
        fclose(arquivo_dados);
        break;
    }

    case 3:
    { // 3: QuickSort Externo
        printf("Executando QuickSort Externo...\n");

        // Nome do arquivo de saída
        const char *output_filename = "data/resultados/ordenacao.bin";
        FILE *output_file = fopen(output_filename, "w+b");
        if (output_file == NULL)
        {
            fprintf(stderr, "Erro ao criar arquivo de saida: '%s'\n", output_filename);
            fclose(arquivo_dados);
            return 1;
        }

        // Copia os registros do arquivo de entrada para o de saída
        TipoRegistro reg;
        long count = 0;
        rewind(arquivo_dados); // Retorna ao início do arquivo de entrada
        while (fread(&reg, sizeof(TipoRegistro), 1, arquivo_dados) == 1 && count < quantidade_registros)
        {
            if (fwrite(&reg, sizeof(TipoRegistro), 1, output_file) != 1)
            {
                fprintf(stderr, "Erro ao escrever registro durante a copia.\n");
                fclose(output_file);
                fclose(arquivo_dados);
                return 1;
            }
            count++;
        }
        fclose(arquivo_dados);

        quicksort_externo(output_file, 0, quantidade_registros - 1);

        fclose(output_file);

        break;
    }
    }

    // Calcula o tempo total de execução da ordenação
    double tempo_execucao = finalizar_tempo(inicio_tempo);

    // Exibe resultados do experimento
    printf("\n--- Resultados do Experimento ---\n");
    printf("Transferencias (I/O): %ld\n", g_io_transferencias);
    printf("Comparacoes: %ld\n", g_comparacoes_chaves);
    printf("Tempo de Execucao: %.4f segundos\n", tempo_execucao);

    // Converte arquivos binários para arquivos texto para análise posterior
    converterBinarioParaTexto("./data/resultados/ordenacao.bin", "./data/resultados/ordenado.txt", quantidade_registros);
    converterBinarioParaTexto(filename, "./data/resultados/preArquivo.txt", quantidade_registros);

    // Se ativado o modo debug, exibe o conteúdo do arquivo ordenado na tela
    if (exibir_chaves_debug)
    {
        printf("\n--- Conteudo do Arquivo Ordenado (para debug) ---\n");
        // Reabre o arquivo de saída ordenado para depuração
        FILE *debug_file = fopen("./data/resultados/ordenacao.bin", "rb");
        if (debug_file == NULL)
        {
            fprintf(stderr, "Erro: Nao foi possivel abrir o arquivo de saida para debug.\n");
            return 1;
        }

        TipoRegistro temp_reg;
        long long count = 0;
        rewind(debug_file);
        while (fread(&temp_reg, sizeof(TipoRegistro), 1, debug_file) == 1 && count < quantidade_registros)
        {
            printf("Inscricao: %lld, Nota: %.2f, Estado: '%s', Cidade: '%s', Curso: '%s'\n",
                   temp_reg.inscricao, temp_reg.nota, temp_reg.estado, temp_reg.cidade, temp_reg.curso);
            count++;
        }
        printf("\n--- Leitura de %lld registros concluída ---\n", count);
        fclose(debug_file);
    }

    return 0;
}