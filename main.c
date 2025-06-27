// main.c

#include "analex.h"
#include "anasint.h"

// --- Variáveis Globais Definidas Aqui ---
TOKEN t;
FILE *fd;
int contLinha = 1;
char TABS[200] = ""; // Variável para controlar a indentação da árvore

void main()
{
    if ((fd = fopen("programa_cshort.txt", "r")) == NULL)
    {
        printf("Erro: Arquivo de entrada 'programa_cshort.txt' nao encontrado.\n");
        return;
    }

    printf("Iniciando analise sintatica...\n");
    printf("-------------------------------------------\n");

    // Adicionamos um cabeçalho para o fluxo de tokens
    printf("FLUXO DE TOKENS CONSUMIDOS:\n");

    Prog(); // Ponto de partida da análise sintática

    printf("\n\n-------------------------------------------\n");
    printf("ARVORE SINTATICA:\n\n");
    // A árvore será impressa conforme o analisador roda.
    // Esta mensagem pode ser removida se preferir.

    printf("\n-------------------------------------------\n");
    printf("Analise sintatica concluida com sucesso!\n");

    fclose(fd);
}