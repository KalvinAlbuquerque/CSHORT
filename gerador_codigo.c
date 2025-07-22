#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gerador_codigo.h"

#define MAX_CODIGO 1000
#define TAM_LINHA 100

// Buffer para armazenar as instruções da máquina de pilha
static char codigo[MAX_CODIGO][TAM_LINHA];
static int linha_atual = 0;

// Contador para geração de rótulos únicos
static int contador_rotulo = 0;

// Gera uma instrução e adiciona ao buffer
void gera(char *instrucao) {
    if (linha_atual < MAX_CODIGO) {
        strncpy(codigo[linha_atual], instrucao, TAM_LINHA);
        linha_atual++;
    } else {
        fprintf(stderr, "Erro: limite de instruções excedido.\n");
    }
}

// Retorna um novo número de rótulo
int novo_rotulo() {
    return contador_rotulo++;
}

// Gera um rótulo (ex: "L1:") como uma instrução
void gera_rotulo(int r) {
    char rotulo[TAM_LINHA];
    snprintf(rotulo, sizeof(rotulo), "L%d:", r);
    gera(rotulo);
}

// Salva as instruções da máquina de pilha em um arquivo .txt
void salvar_codigo_em_arquivo(const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo para escrita");
        return;
    }

    for (int i = 0; i < linha_atual; i++) {
        fprintf(arquivo, "%s\n", codigo[i]);
    }

    fclose(arquivo);
    printf("Código de máquina salvo em: %s\n", nome_arquivo);
}
