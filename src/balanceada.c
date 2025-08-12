#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#include "./include/common_types.h"
#include "./include/utils.h"

// Define a quantidade de fitas
#define MEM_MAX 20 // Memória interna disponível (em registros)
#define F 20       // Número de fitas de entrada e saída
#define FF 40      // Número total de fitas

// Comparador para qsort (Ordenação interna)
int comparadorNotas(const void *a, const void *b)
{
    TipoRegistro *reg_a = (TipoRegistro *)a;
    TipoRegistro *reg_b = (TipoRegistro *)b;
    if (reg_a->nota < reg_b->nota)
        return -1;
    if (reg_a->nota > reg_b->nota)
        return 1;
    return 0;
}

// Retorna 1 (true) se todas as fitas (blocos) estão esgotadas
short todosBlocosEsgotados(short ativas[], int n)
{
    for (int i = 0; i < n; i++)
    {
        if (ativas[i] != -1)
            return 0;
    }
    return 1;
}

// Verifica se entre n fitas (a partir de índice inicio) só existe uma com blocos > 0
// Retorna índice dessa fita se for única, ou -1 caso contrário
int restaUmaFitaPreenchida(int nBlocos[], int n, int inicio)
{
    int preenchida = -1;
    for (int i = 0; i < n; i++)
    {
        if (nBlocos[inicio + i] > 0)
        {
            if (preenchida == -1)
                preenchida = i;
            else
                return -1; // achou mais de uma fita preenchida
        }
    }
    // Retorna índice absoluto da fita única preenchida ou -1 se nenhuma
    return (preenchida == -1) ? preenchida : inicio + preenchida;
}

// Heapify adaptado para heap mínimo considerando um array de registros e "congelados"
void heapify(TipoRegistro arr[], int n, int i, short congelados[])
{
    int menor = i;
    int esq = 2 * i + 1;
    int dir = 2 * i + 2;
    
    // Verifica filho esquerdo não congelado e menor que atual
    if (esq < n && congelados[esq] == 0 && arr[esq].nota < arr[menor].nota)
    {
        menor = esq;
    }
    // Verifica filho direito não congelado e menor que atual menor
    if (dir < n && congelados[dir] == 0 && arr[dir].nota < arr[menor].nota)
    {
        menor = dir;
    }

    // Se menor mudou, troca e chama recursivamente heapify para manter heap mínimo
    if (menor != i)
    {
        TipoRegistro tmp = arr[i];
        arr[i] = arr[menor];
        arr[menor] = tmp;

        short tmpc = congelados[i];
        congelados[i] = congelados[menor];
        congelados[menor] = tmpc;

        heapify(arr, n, menor, congelados);
    }
}

// --- INTERCALAÇÃO BALANCEADA M-WAY ---
void intercalacao_balanceada(FILE **fitas, long n_registros, int n_blocos_iniciais[FF])
{
    short grupo_entrada = 0; // Indica qual grupo de fitas está como entrada (0 ou 1)
    int n_blocos[FF];
    memcpy(n_blocos, n_blocos_iniciais, sizeof(int) * FF); // Copia contagem de blocos para controle interno

    // Enquanto houver mais de uma fita com blocos para processar
    while (restaUmaFitaPreenchida(n_blocos, F, grupo_entrada * F) == -1)
    {

        int entrada_inicio = grupo_entrada * F; // índice inicial do grupo de entrada
        int saida_inicio = (1 - grupo_entrada) * F; // índice inicial do grupo de saída

        // Limpa as fitas de saída
        for (int i = 0; i < F; i++)
        {
            rewind(fitas[saida_inicio + i]);
            ftruncate(fileno(fitas[saida_inicio + i]), 0);
            n_blocos[saida_inicio + i] = 0;
        }

        // Reposiciona as fitas de entrada para início
        for (int i = 0; i < F; i++)
        {
            rewind(fitas[entrada_inicio + i]);
        }

        int fita_saida_atual = saida_inicio;
        int blocos_restantes_fita[F];
        memcpy(blocos_restantes_fita, &n_blocos[entrada_inicio], sizeof(int) * F);

        // Intercala todos os blocos do grupo de entrada
        bool ainda_ha_blocos_para_intercalar = true;
        while (ainda_ha_blocos_para_intercalar)
        {
            TipoRegistro registros_na_mem[F];
            short ativas[F];
            ainda_ha_blocos_para_intercalar = false;

            // Pega o primeiro registro de cada um dos blocos que ainda não foram intercalados
            for (int i = 0; i < F; i++)
            {
                if (blocos_restantes_fita[i] > 0)
                {
                    incrementar_io();
                    if (fread(&registros_na_mem[i], sizeof(TipoRegistro), 1, fitas[entrada_inicio + i]) == 1)
                    {
                        ativas[i] = 1;
                        ainda_ha_blocos_para_intercalar = true;
                    }
                    else
                    {
                        ativas[i] = -1;// fita esgotada
                    }
                }
                else
                {
                    ativas[i] = -1;// bloco esgotado
                }
            }

            if (!ainda_ha_blocos_para_intercalar)
                break; // encerra se não há blocos para processar

            // Inicia um novo bloco de saída
            n_blocos[fita_saida_atual]++;
            while (true)
            {
                int menor_idx = -1;
                // Busca o menor registro entre os ativos para escrever no bloco de saída
                for (int i = 0; i < F; i++)
                {
                    if (ativas[i] != -1)
                    {
                        incrementar_comparacao();
                        if (menor_idx == -1 || registros_na_mem[i].nota < registros_na_mem[menor_idx].nota)
                        {
                            menor_idx = i;
                        }
                    }
                }

                if (menor_idx == -1)
                {
                    break; // nenhum ativo restante, fim do bloco
                }

                incrementar_io();
                fwrite(&registros_na_mem[menor_idx], sizeof(TipoRegistro), 1, fitas[fita_saida_atual]);

                TipoRegistro novo;
                incrementar_io();
                // Tenta ler próximo registro da fita que forneceu o menor
                if (fread(&novo, sizeof(TipoRegistro), 1, fitas[entrada_inicio + menor_idx]) == 1)
                {
                    // Se o próximo registro quebra a ordem crescente do bloco, bloqueia esta fita no bloco atual
                    if (novo.nota < registros_na_mem[menor_idx].nota)
                    {

                        ativas[menor_idx] = -1; // fita desativada neste bloco
                        // Volta um registro para ler na próxima intercalação
                        fseek(fitas[entrada_inicio + menor_idx], -sizeof(TipoRegistro), SEEK_CUR); // Volta para ler na próxima intercalação
                    }
                    else
                    {
                        registros_na_mem[menor_idx] = novo; // atualiza o registro lido para próximo ciclo
                    }
                }
                else
                {
                    // Fim da fita, não há mais blocos para ler.
                    ativas[menor_idx] = -1;
                    blocos_restantes_fita[menor_idx]--;
                }
            }
            // Avança para próxima fita de saída
            fita_saida_atual = (fita_saida_atual + 1 - saida_inicio) % F + saida_inicio;
        }

        grupo_entrada = 1 - grupo_entrada; // alterna entre grupos de entrada e saída
    }

    // Após finalização, copia conteúdo da fita que sobrou para arquivo final de ordenação
    int f_final = restaUmaFitaPreenchida(n_blocos, F, grupo_entrada * F);
    if (f_final != -1)
    {
        rewind(fitas[f_final]);
        FILE *out = fopen("data/resultados/ordenacao.bin", "wb");
        if (!out)
        {
            perror("Erro ao criar arquivo de saida");
            return;
        }
        TipoRegistro reg;
        while (fread(&reg, sizeof(TipoRegistro), 1, fitas[f_final]) == 1)
        {
            fwrite(&reg, sizeof(TipoRegistro), 1, out);
        }
        fclose(out);
    }
}

// --- MÉTODO 1: INTERCALAÇÃO COM ORDENAÇÃO INTERNA ---
void metodo_intercalacao_ordenacao(const char *entrada, long n_registros)
{

    mkdir("data/fitas", 0777);
    mkdir("data/resultados",0777);

    FILE *in = fopen(entrada, "rb");
    if (!in)
    {
        perror("Erro ao abrir arquivo de entrada");
        return;
    }

    FILE *fitas[FF];
    char nome[128];
    for (int i = 0; i < FF; i++)
    {
        sprintf(nome, "data/fitas/fita_%02d.tmp", i);
        fitas[i] = fopen(nome, "wb+");
        if (!fitas[i])
        {
            perror("Erro ao criar fita");
            fclose(in);
            return;
        }
    }

    TipoRegistro buffer[MEM_MAX]; // buffer para ler blocos em memória
    int n_blocos[FF] = {0}; // contador de blocos gerados em cada fita
    int fita_atual = 0; // fita atual para gravação
    long registros_restantes = n_registros;
    long total_lidos = 0;

    while (registros_restantes > 0)
    {
        incrementar_io();
        int lidos = fread(buffer, sizeof(TipoRegistro), MEM_MAX, in);
        if (lidos == 0)
            break;

        // Ordena bloco em memória pelo campo nota
        qsort(buffer, lidos, sizeof(TipoRegistro), comparadorNotas);

        incrementar_io();
        // Grava bloco ordenado na fita atual
        fwrite(buffer, sizeof(TipoRegistro), lidos, fitas[fita_atual]);

        n_blocos[fita_atual]++;
        registros_restantes -= lidos;
        total_lidos += lidos;
        fita_atual = (fita_atual + 1) % F; //alterna fita para próximo bloco
    }

    fclose(in);

    // Rewind nas fitas para a fase de intercalação
    for (int i = 0; i < F; i++)
    {
        rewind(fitas[i]);
    }

    intercalacao_balanceada(fitas, n_registros, n_blocos);

    for (int i = 0; i < FF; i++)
        fclose(fitas[i]);
}

// --- MÉTODO 2: INTERCALAÇÃO COM SELEÇÃO POR SUBSTITUIÇÃO ---
void metodo_intercalacao_selecao(const char *entrada, long n_registros)
{
    mkdir("data/fitas", 0777);
    mkdir("data/resultados",0777);

    FILE *in = fopen(entrada, "rb");
    if (!in)
    {
        perror("Erro ao abrir arquivo de entrada");
        return;
    }

    FILE *fitas[FF];
    char nome[128];
    for (int i = 0; i < FF; i++)
    {
        sprintf(nome, "data/fitas/fita_%02d.tmp", i);
        fitas[i] = fopen(nome, "wb+");
        if (!fitas[i])
        {
            perror("Erro ao criar fita");
            fclose(in);
            return;
        }
    }

    TipoRegistro memoria[MEM_MAX]; // buffer de registros em memória
    short congelados[MEM_MAX]; // vetor para marcar registros congelados
    int n_blocos[FF] = {0};// contador de blocos por fita
    int fita_atual = 0;// fita atual para gravação
    int lidos_memoria;
    bool fim_arquivo = false;
    int blocos_gerados_total = 0;

    // VARIÁVEL PARA RASTREAR O ÚLTIMO REGISTRO ESCRITO
    incrementar_io();
    lidos_memoria = fread(memoria, sizeof(TipoRegistro), MEM_MAX, in);
    if (lidos_memoria < MEM_MAX)
        fim_arquivo = true;
    for (int i = 0; i < lidos_memoria; i++)
        congelados[i] = 0;

    // Constroi heap mínimo a partir do array memória
    for (int i = (lidos_memoria / 2) - 1; i >= 0; i--)
    {
        heapify(memoria, lidos_memoria, i, congelados);
    }
    
    // Enquanto houver registros na memória para processar
    while (lidos_memoria > 0)
    {
        
        int menor_idx = 0;
        TipoRegistro menor = memoria[menor_idx];

        // Escreve o menor registro do heap na fita atual
        incrementar_io();
        fwrite(&menor, sizeof(TipoRegistro), 1, fitas[fita_atual]);

        TipoRegistro novo;
        bool leu_novo = false;
        // Lê um novo registro do arquivo, se possível
        if (!fim_arquivo)
        {
            incrementar_io();
            if (fread(&novo, sizeof(TipoRegistro), 1, in) == 1)
            {
                leu_novo = true;
            }
            else
            {
                fim_arquivo = true;
            }
        }

        if (leu_novo)
        {
            incrementar_comparacao();
            // Se o novo registro mantém a ordenação crescente
            if (novo.nota >= menor.nota)
            {
                memoria[menor_idx] = novo;
                congelados[menor_idx] = 0;
            }
            else
            {
                // Caso contrário, move o registro para o final e congela para bloco atual
                memoria[menor_idx] = memoria[lidos_memoria - 1];
                memoria[lidos_memoria - 1] = novo;
                congelados[lidos_memoria - 1] = 1; // Congela na última posição
            }
        }
        else
        {   // Fim do arquivo: remove registro do heap, ajusta congelados e decrementa contagem
            memoria[menor_idx] = memoria[lidos_memoria - 1];
            congelados[menor_idx] = congelados[lidos_memoria - 1];
            congelados[lidos_memoria - 1] = 0;
            lidos_memoria--;
        }

        if (lidos_memoria > 0)
        {
            heapify(memoria, lidos_memoria, 0, congelados);
        }
        else
        {
            // Quando heap está vazio, reinicia com registros congelados para formar novo bloco
            int novos_lidos = 0;
            for (int i = 0; i < MEM_MAX; i++)
            {
                if (congelados[i] == 1)
                {
                    memoria[novos_lidos] = memoria[i];
                    congelados[novos_lidos] = 0;
                    novos_lidos++;
                }
            }
            lidos_memoria = novos_lidos;

            if (lidos_memoria > 0)
            {
                // Reconstrói heap
                for (int i = (lidos_memoria / 2) - 1; i >= 0; i--)
                {
                    heapify(memoria, lidos_memoria, i, congelados);
                }
            }
            n_blocos[fita_atual]++; // Incrementa contador de blocos gerados
            blocos_gerados_total++;
            fita_atual = (fita_atual + 1) % F; // Avança para próxima fita
        }
    }

    // Escreve os registros restantes na memória para a fita atual
    while (lidos_memoria > 0)
    {
        TipoRegistro menor = memoria[0];
        incrementar_io();
        fwrite(&menor, sizeof(TipoRegistro), 1, fitas[fita_atual]);

        memoria[0] = memoria[lidos_memoria - 1];
        congelados[0] = congelados[lidos_memoria - 1];
        lidos_memoria--;

        if (lidos_memoria > 0)
            heapify(memoria, lidos_memoria, 0, congelados);
    }
    n_blocos[fita_atual]++;
    blocos_gerados_total++;

    fclose(in);

    // Reposiciona todas as fitas para a fase de intercalação final
    for (int i = 0; i < FF; i++)
    {
        rewind(fitas[i]);
    }

    // Realiza intercalação balanceada para combinar os blocos gerados
    intercalacao_balanceada(fitas, n_registros, n_blocos);

    // Fecha todas as fitas temporárias
    for (int i = 0; i < FF; i++)
        fclose(fitas[i]);
}