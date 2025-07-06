#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

// Definição comum para os Dados do Aluno
typedef struct {
    long long inscricao;
    float nota;
    char estado[3];   // 2 caracteres para o estado + 1 para '\0'
    char cidade[51];  // 50 caracteres para a cidade + 1 para '\0'
    char curso[31];   // 30 caracteres para o curso + 1 para '\0'
} TipoRegistro;

// Função auxiliar para remover espaços em branco do final de uma string
void trim_trailing_spaces(char *s) {
    int i = strlen(s) - 1;
    while (i >= 0 && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r')) {
        s[i] = '\0';
        i--;
    }
}

#endif // COMMON_TYPES_H