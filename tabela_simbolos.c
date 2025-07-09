/**
 * @file tabela_simbolos.c
 * @brief Implementação do gerenciador da Tabela de Símbolos.
 *
 * Este arquivo contém todas as funções para manipular a Tabela de Símbolos,
 * uma estrutura de dados essencial para o compilador que armazena informações
 * sobre os identificadores (variáveis, funções, etc.) do código-fonte.
 * A tabela é implementada como uma pilha para gerenciar escopos de forma eficaz.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabela_simbolos.h"
#include "analex.h"

/** A instância global da tabela de símbolos, acessível por todo o módulo. */
Tabela tabela;

/** Vetor de strings para mapear o enum de escopo para texto legível. */
char *T_escopo[] = {
    [GLOBAL] = "Global",
    [LOCAL] = "Local"
};

/** Vetor de strings para mapear o enum de tipo de dado para texto legível. */
char *T_tipo[] = {
    [INT_] = "Int",
    [REAL_] = "Real",
    [CHAR_] = "Char",
    [BOOL_] = "Bool",
    [NA_TIPO] = "Void"
};

/** Vetor de strings para mapear o enum de categoria de identificador para texto legível. */
char *T_IdCategoria[] = {
    [VAR_GLOBAL] = "Var_G",
    [VAR_LOCAL] = "Var_L",
    [PROC] = "Proc",
    [PROC_PAR] = "Param",
    [PROT_] = "Prot"
};

/** Vetor de strings para mapear o enum de status "zumbi" para texto legível. */
char *T_zumbi[] = {
    [VIVO] = "0",
    [ZUMBI_] = "1",
};

// ... (outros vetores de mapeamento seguem o mesmo padrão) ...


/**
 * @brief Insere uma nova entrada (símbolo) na tabela.
 *
 * Algoritmo:
 * 1. Chama a função `buscaDeclRep` para garantir que o símbolo não está sendo redeclarado ilegalmente.
 * 2. Adiciona a estrutura `TokenInfo` fornecida na próxima posição livre da tabela (o topo da pilha).
 * 3. Incrementa o ponteiro do topo (`tabela.topo`), efetivamente "empilhando" o novo símbolo.
 * 4. Chama `printarTabela` para depuração, mostrando o estado atual da tabela.
 *
 * @param token A estrutura contendo todas as informações do símbolo a ser inserido.
 */
void inserirNaTabela(TokenInfo token){
    buscaDeclRep(token); // Verifica Repetição de lexema
    tabela.tokensTab[tabela.topo] = token;
    tabela.topo++;
    printarTabela(-1);
}

/**
 * @brief Busca por declarações repetidas de um mesmo identificador.
 *
 * Algoritmo:
 * Realiza uma varredura linear (O(n)) em toda a tabela. Para cada entrada,
 * compara o lexema (nome) com o do novo token. Se os nomes forem iguais,
 * aplica regras específicas para determinar se é uma redeclaração ilegal
 * (ex: duas variáveis globais com o mesmo nome, ou dois parâmetros vivos
 * no mesmo escopo). Se uma redeclaração ilegal for encontrada, a função
 * `error` é chamada para encerrar a compilação.
 *
 * @param token As informações do novo símbolo que está sendo declarado.
 */
void buscaDeclRep(TokenInfo token){
    for(int i = 0; i < tabela.topo; i++){
        if(strcmp(token.lexema, tabela.tokensTab[i].lexema) == 0){
            if(tabela.tokensTab[i].idcategoria == PROC && token.idcategoria == PROC) error("Redeclaração de procedimento encontrada");
            if(tabela.tokensTab[i].idcategoria == VAR_LOCAL && token.idcategoria == VAR_LOCAL) error("Redeclaração de variável encontrada");
            if(tabela.tokensTab[i].idcategoria == VAR_GLOBAL && token.idcategoria == VAR_GLOBAL) error("Redeclaração de variável global encontrada");
            // A condição de "zumbi" impede que parâmetros de escopos antigos causem erro de redeclaração.
            if(tabela.tokensTab[i].zumbi == VIVO && strcmp(token.lexema, tabela.tokensTab[i].lexema) == 0) error("Redeclaração de parâmetro encontrada");
        }
    }
}

/**
 * @brief Busca a posição da declaração mais recente de um lexema.
 *
 * Algoritmo:
 * Realiza uma busca linear (O(n)) na tabela, mas começando do fim (`tabela.topo-1`)
 * para o início. Esta abordagem de "pilha" é essencial para o tratamento de escopos.
 * Ela garante que a declaração mais próxima (mais "no topo da pilha") seja encontrada
 * primeiro (ex: uma variável local antes de uma global com o mesmo nome).
 * Símbolos marcados como "ZUMBI_" são ignorados, pois estão fora de escopo.
 *
 * @param lexema O nome (string) do identificador a ser buscado.
 * @return O índice do lexema na tabela se encontrado e ativo; -1 caso contrário.
 */
int buscaLexPos(char *lexema){
    for(int i = tabela.topo - 1; i >= 0; i--){
        if(strcmp(lexema, tabela.tokensTab[i].lexema) == 0 && tabela.tokensTab[i].zumbi != ZUMBI_){
            return i;
        }
    }
    return -1;
}

/**
 * @brief Imprime o conteúdo atual da tabela de símbolos de forma formatada.
 *
 * Esta é uma função de depuração visual. Ela percorre a tabela de símbolos
 * e imprime colunas selecionadas de cada entrada, usando os vetores de
 * mapeamento (ex: `T_escopo`) para exibir texto em vez de números.
 * A chamada `getchar()` ao final pausa a execução, permitindo que o
 * desenvolvedor analise o estado da tabela passo a passo.
 *
 * @param pos Posição a ser destacada (atualmente não utilizado, -1 por padrão).
 */
void printarTabela(int pos) {
    TokenInfo aux;
    printf("\n");
    printf("+-------------------------------+----------+-----------+-------+-------+\n");
    printf("| Lexema                        | escopo   | classe    | tipo  | zumbi |\n");
    printf("+-------------------------------+----------+-----------+-------+-------+\n");

    if (tabela.topo == 0) {
        printf("| Tabela de Símbolos está vazia.                                       |\n");
    }

    for (int i = 0; i < tabela.topo; i++) {
        aux = tabela.tokensTab[i];
        printf("| %-30s|", aux.lexema);
        printf(" %-9s|", T_escopo[aux.escopo]);
        printf(" %-10s|", T_IdCategoria[aux.idcategoria]);
        printf(" %-6s|", T_tipo[aux.tipo]);
        printf(" %-6s|", T_zumbi[aux.zumbi]);
        printf("\n");
    }

    printf("+-------------------------------+----------+-----------+-------+-------+\n");
    printf("Pressione Enter para continuar...\n");
    getchar();
}

/**
 * @brief Remove o elemento do topo da tabela de símbolos (operação de "pop").
 *
 * Esta é uma função de baixo nível que simplesmente decrementa o ponteiro
 * do topo da pilha, efetivamente removendo o último símbolo inserido.
 */
void removerDaTabela(){
    if (tabela.topo > 0) {
        tabela.topo--;
    } else {
        printf("Tabela já está vazia.\n");
    }
}

/**
 * @brief Apaga todo o conteúdo da tabela de símbolos.
 *
 * Utiliza `memset` para zerar a memória do vetor de tokens e redefine o
 * topo para 0, restaurando a tabela ao seu estado inicial.
 */
void limparTabela() {
    memset(tabela.tokensTab, 0, sizeof(tabela.tokensTab));
    tabela.topo = 0;
}

/**
 * @brief Reseta uma estrutura TokenInfo para um estado limpo (zerado).
 * @param token Um ponteiro para a estrutura TokenInfo a ser resetada.
 */
void resetTokenInfo(TokenInfo *token) {
    memset(token, 0, sizeof(TokenInfo));
    memset(token->lexema, 0, sizeof(token->lexema));
}

/**
 * @brief Marca os parâmetros de uma função como "zumbis" ao sair de seu escopo.
 *
 * Algoritmo:
 * Ao final da análise do corpo de uma função, seus parâmetros se tornam inacessíveis.
 * Esta função percorre os parâmetros associados à função (`procPos`) e muda seu
 * status para `ZUMBI_`. Isso os "desativa" para futuras buscas de nome, mas mantém
 * suas informações na tabela para outras possíveis análises do compilador.
 *
 * @param procPos O índice inicial do procedimento na tabela.
 */
void matarZumbis(int procPos){
    procPos++;
    while(1){
        if(tabela.tokensTab[procPos].idcategoria != PROC_PAR) break;
        tabela.tokensTab[procPos].zumbi = ZUMBI_;
        printarTabela(procPos);
        procPos++;
    }
}

/**
 * @brief Remove todas as variáveis locais do escopo atual.
 *
 * Algoritmo:
 * Implementa a operação de "saída de escopo". A função olha para o símbolo no topo
 * da pilha da tabela. Se for uma variável local (`VAR_LOCAL`), ela o remove
 * chamando `removerDaTabela()`. O processo se repete até que o topo da pilha
 * não seja mais uma variável local, efetivamente limpando todo o escopo local.
 */
void retirarLocais(){
    while(1){
        if(tabela.tokensTab[tabela.topo-1].idcategoria != VAR_LOCAL) break;
        removerDaTabela();
        printarTabela(-1);
    }
}

/**
 * @brief Busca por um símbolo e retorna sua estrutura de dados completa.
 *
 * Esta função é um invólucro (wrapper) para `buscaLexPos`. Ela chama a função
 * de busca e, se o símbolo não for encontrado (`pos < 0`), ela dispara um erro fatal.
 * Caso contrário, retorna a estrutura `TokenInfo` do símbolo encontrado.
 *
 * @param lexema O nome do identificador a ser buscado.
 * @return A estrutura TokenInfo completa do símbolo encontrado.
 */
TokenInfo buscaDecl(char *lexema){
    int pos = buscaLexPos(lexema);
    if(pos < 0)  error("Declaração não encontrada");
    return tabela.tokensTab[pos];
}