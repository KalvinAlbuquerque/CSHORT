#include "gerador_codigo.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

// Ponteiro estático para o arquivo, visível apenas neste módulo.
static FILE* arquivo_saida;
// Contador estático para garantir rótulos únicos.
static int contador_rotulo = 0;

void inicializar_gerador(const char* nome_arquivo) {
    arquivo_saida = fopen(nome_arquivo, "w");
    if (arquivo_saida == NULL) {
        printf("Erro: Nao foi possivel criar o arquivo de saida '%s'.\n", nome_arquivo);
        exit(1);
    }
}

void finalizar_gerador() {
    if (arquivo_saida != NULL) {
        fclose(arquivo_saida);
        arquivo_saida = NULL;
    }
}

void gerar_codigo(const char* formato, ...) {
    if (arquivo_saida == NULL) {
        return; // Não faz nada se o gerador não foi inicializado.
    }
    va_list args;
    va_start(args, formato);
    vfprintf(arquivo_saida, formato, args);
    fprintf(arquivo_saida, "\n"); // Adiciona uma nova linha após cada instrução
    va_end(args);
}

void novo_rotulo(char* rotulo) {
    // Gera um rótulo no formato "L<numero>"
    sprintf(rotulo, "L%d", contador_rotulo++);
}