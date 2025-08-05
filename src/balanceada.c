#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>

#include "./include/common_types.h"
#include "./include/utils.h"
#include "./include/balanceada.h"

#define MEM_MAX 20
#define F 20
#define FF 40

// --- FUNÇÕES AUXILIARES ---

short todosBlocosEsgotados(short ativas[], int n) {
    for (int i = 0; i < n; i++) {
        if (ativas[i] != -1) return 0;
    }
    return 1;
}

int restaUmaFitaPreenchida(int nBlocos[], int n, int inicio) {
    int preenchida = -1;
    for (int i = 0; i < n; i++) {
        if (nBlocos[inicio + i] > 0) {
            if (preenchida == -1) preenchida = i;
            else return -1;
        }
    }
    return (preenchida == -1) ? preenchida : inicio + preenchida;
}

void heapify(TipoRegistro arr[], int n, int i, short congelados[]) {
    int menor = i;
    int esq = 2 * i + 1;
    int dir = 2 * i + 2;

    if (esq < n && congelados[esq] == 0 && arr[esq].nota < arr[menor].nota) {
        menor = esq;
    }
    if (dir < n && congelados[dir] == 0 && arr[dir].nota < arr[menor].nota) {
        menor = dir;
    }

    if (menor != i) {
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
void intercalacao_balanceada(FILE **fitas, long n_registros, int n_blocos_iniciais[FF]) {
    short grupo_entrada = 0;
    int n_blocos[FF];
    memcpy(n_blocos, n_blocos_iniciais, sizeof(int) * FF);

    while (restaUmaFitaPreenchida(n_blocos, F, grupo_entrada * F) == -1) {
        int entrada_inicio = grupo_entrada * F;
        int saida_inicio = (1 - grupo_entrada) * F;

        for (int i = 0; i < F; i++) {
            rewind(fitas[saida_inicio + i]);
            ftruncate(fileno(fitas[saida_inicio + i]), 0);
            n_blocos[saida_inicio + i] = 0;
        }

        for (int i = 0; i < F; i++) {
            rewind(fitas[entrada_inicio + i]);
        }
        
        int blocos_restantes[F];
        memcpy(blocos_restantes, &n_blocos[entrada_inicio], sizeof(int) * F);

        int fita_saida_atual = saida_inicio;

        while (true) {
            TipoRegistro registros_na_mem[F];
            short ativas[F];
            TipoRegistro ultimo_escrito[F]; // Novo array para rastrear o último registro escrito de cada fita

            for (int i = 0; i < F; i++) {
                ativas[i] = -1;
                if (blocos_restantes[i] > 0) {
                    incrementar_io();
                    if (fread(&registros_na_mem[i], sizeof(TipoRegistro), 1, fitas[entrada_inicio + i]) == 1) {
                        ativas[i] = 1;
                        ultimo_escrito[i] = registros_na_mem[i]; // Inicializa o último escrito para o primeiro registro do bloco
                    }
                }
            }
            if (todosBlocosEsgotados(ativas, F)) break;

            while (true) {
                int menor_idx = -1;
                for (int i = 0; i < F; i++) {
                    if (ativas[i] != -1) {
                        incrementar_comparacao();
                        if (menor_idx == -1 || registros_na_mem[i].nota < registros_na_mem[menor_idx].nota) {
                            menor_idx = i;
                        }
                    }
                }
                if (menor_idx == -1) break;

                incrementar_io();
                fwrite(&registros_na_mem[menor_idx], sizeof(TipoRegistro), 1, fitas[fita_saida_atual]);

                TipoRegistro novo;
                incrementar_io();
                if (fread(&novo, sizeof(TipoRegistro), 1, fitas[entrada_inicio + menor_idx]) == 1) {
                    incrementar_comparacao();
                    if (novo.nota < registros_na_mem[menor_idx].nota) {
                         ativas[menor_idx] = -1; // Bloco finalizado
                         blocos_restantes[menor_idx]--;
                    } else {
                        registros_na_mem[menor_idx] = novo;
                        // Não atualizamos `ultimo_escrito` aqui, a lógica de intercalação deve gerenciar a ordem do fluxo.
                    }
                } else {
                    ativas[menor_idx] = -1;
                    blocos_restantes[menor_idx]--;
                }
            }

            n_blocos[saida_inicio + (fita_saida_atual - saida_inicio)]++;
            fita_saida_atual = (fita_saida_atual + 1 - saida_inicio) % F + saida_inicio;
        }

        grupo_entrada = 1 - grupo_entrada;
    }
    
    int f_final = restaUmaFitaPreenchida(n_blocos, F, grupo_entrada * F);
    if (f_final != -1) {
        rewind(fitas[f_final]);
        FILE *out = fopen("data/resultados/ordenacao.bin", "wb");
        if (!out) { perror("Erro ao criar arquivo de saída"); return; }
        TipoRegistro reg;
        while (fread(&reg, sizeof(TipoRegistro), 1, fitas[f_final]) == 1) {
            fwrite(&reg, sizeof(TipoRegistro), 1, out);
        }
        fclose(out);
    }
}

// --- MÉTODO 1: INTERCALAÇÃO COM ORDENAÇÃO INTERNA ---
void metodo_intercalacao_ordenacao(const char *entrada, long n_registros) {
    mkdir("data/fitas");
    mkdir("data/resultados");

    FILE *in = fopen(entrada, "rb");
    if (!in) { perror("Erro ao abrir arquivo"); return; }

    FILE *fitas[FF];
    char nome[128];
    for (int i = 0; i < FF; i++) {
        sprintf(nome, "data/fitas/fita_%02d.tmp", i);
        fitas[i] = fopen(nome, "wb+");
        if (!fitas[i]) { perror("Erro ao criar fita"); fclose(in); return; }
    }

    TipoRegistro buffer[MEM_MAX];
    int n_blocos[FF] = {0};
    int fita_atual = 0;
    long registros_restantes = n_registros;

    while (registros_restantes > 0) {
        incrementar_io();
        int lidos = fread(buffer, sizeof(TipoRegistro), MEM_MAX, in);
        if (lidos == 0) break;

        qsort(buffer, lidos, sizeof(TipoRegistro), comparadorNotas);

        incrementar_io();
        fwrite(buffer, sizeof(TipoRegistro), lidos, fitas[fita_atual]);

        n_blocos[fita_atual]++;
        registros_restantes -= lidos;
        fita_atual = (fita_atual + 1) % F;
    }
    fclose(in);

    intercalacao_balanceada(fitas, n_registros, n_blocos);

    for (int i = 0; i < FF; i++)
        fclose(fitas[i]);
}

// --- MÉTODO 2: INTERCALAÇÃO COM SELEÇÃO POR SUBSTITUIÇÃO ---
void metodo_intercalacao_selecao(const char *entrada, long n_registros) {
    mkdir("data/fitas");
    mkdir("data/resultados");

    FILE *in = fopen(entrada, "rb");
    if (!in) { perror("Erro ao abrir arquivo"); return; }

    FILE *fitas[FF];
    char nome[128];
    for (int i = 0; i < FF; i++) {
        sprintf(nome, "data/fitas/fita_%02d.tmp", i);
        fitas[i] = fopen(nome, "wb+");
        if (!fitas[i]) { perror("Erro ao criar fita"); fclose(in); return; }
    }

    TipoRegistro memoria[MEM_MAX];
    short congelados[MEM_MAX];
    int n_blocos[FF] = {0};
    int fita_atual = 0;
    int lidos_memoria = 0;

    incrementar_io();
    lidos_memoria = fread(memoria, sizeof(TipoRegistro), MEM_MAX, in);
    for (int i = 0; i < lidos_memoria; i++) congelados[i] = 0;

    for (int i = (lidos_memoria / 2) - 1; i >= 0; i--) {
        heapify(memoria, lidos_memoria, i, congelados);
    }
    
    bool fim_arquivo = (lidos_memoria < MEM_MAX);

    while (lidos_memoria > 0) {
        TipoRegistro menor = memoria[0];

        if (congelados[0] == 1) {
            n_blocos[fita_atual]++;
            fita_atual = (fita_atual + 1) % F;

            int novos_lidos = 0;
            for (int i = 0; i < lidos_memoria; i++) {
                if (congelados[i] == 1) {
                    memoria[novos_lidos] = memoria[i];
                    congelados[novos_lidos] = 0;
                    novos_lidos++;
                }
            }
            lidos_memoria = novos_lidos;

            if (lidos_memoria == 0 && !fim_arquivo) {
                incrementar_io();
                int lidos_adicionais = fread(memoria, sizeof(TipoRegistro), MEM_MAX, in);
                lidos_memoria = lidos_adicionais;
                for (int i = 0; i < lidos_memoria; i++) congelados[i] = 0;
                if (lidos_memoria < MEM_MAX) fim_arquivo = true;
            }
            
            for (int i = (lidos_memoria / 2) - 1; i >= 0; i--) {
                heapify(memoria, lidos_memoria, i, congelados);
            }
            if (lidos_memoria > 0) continue;
        }

        incrementar_io();
        fwrite(&menor, sizeof(TipoRegistro), 1, fitas[fita_atual]);

        TipoRegistro novo;
        if (!fim_arquivo) {
            incrementar_io();
            if (fread(&novo, sizeof(TipoRegistro), 1, in) != 1) {
                fim_arquivo = true;
            }
        }
        
        if (!fim_arquivo) {
            incrementar_comparacao();
            if (novo.nota >= menor.nota) {
                memoria[0] = novo;
            } else {
                congelados[0] = 1;
                memoria[0] = novo;
            }
        } else {
            memoria[0] = memoria[lidos_memoria - 1];
            congelados[0] = congelados[lidos_memoria - 1];
            lidos_memoria--;
        }

        if (lidos_memoria > 0)
            heapify(memoria, lidos_memoria, 0, congelados);
    }
    
    while(lidos_memoria > 0) {
        TipoRegistro menor = memoria[0];
        incrementar_io();
        fwrite(&menor, sizeof(TipoRegistro), 1, fitas[fita_atual]);
        memoria[0] = memoria[lidos_memoria-1];
        congelados[0] = congelados[lidos_memoria-1];
        lidos_memoria--;
        if (lidos_memoria > 0)
            heapify(memoria, lidos_memoria, 0, congelados);
    }
    
    n_blocos[fita_atual]++;

    fclose(in);
    
    intercalacao_balanceada(fitas, n_registros, n_blocos);

    for (int i = 0; i < FF; i++)
        fclose(fitas[i]);
}