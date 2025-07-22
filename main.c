#include <stdio.h>
#include "analex.h"
#include "anasint.h"
#include "tabela_simbolos.h"
#include "gerador_codigo.h"

// --- Variáveis Globais Definidas Aqui ---
TOKEN t;
FILE *fd;
int contLinha = 1;
char TABS[200] = ""; // Variável para controlar a indentação da árvore

void main()
{
    // Tenta abrir o arquivo de código-fonte para leitura
    if ((fd = fopen("programa_cshort.txt", "r")) == NULL)
    {
        printf("Erro: Arquivo de entrada 'programa_cshort.txt' nao encontrado.\n");
        printf("Por favor, certifique-se de que o arquivo com o codigo a ser compilado existe no mesmo diretorio.\n");
        return;
    }

    // Inicializa o gerador de código, que criará o arquivo de saída.
    // O código traduzido será salvo em "saida_mp.txt".
    inicializar_gerador("saida_mp.txt");

    printf("Iniciando analise sintatica...\n");
    printf("-------------------------------------------\n");
    printf("ARVORE DE ANALISE SINTATICA:\n\n");

    // Chama a função Prog(), que é o ponto de partida da análise sintática
    // e da geração de código.
    Prog();

    printf("\n-------------------------------------------\n");
    printf("Analise sintatica e geracao de codigo concluidas com sucesso!\n");
    printf("O codigo para a Maquina de Pilha foi salvo em 'saida_mp.txt'.\n");

    // Finaliza o gerador de código, garantindo que o arquivo de saída seja
    // devidamente fechado e salvo.
    finalizar_gerador();

    // Fecha o arquivo de código-fonte.
    fclose(fd);
}