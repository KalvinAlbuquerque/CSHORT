/**
 * @file tabela_simbolos.h
 * @brief Definição da Interface Pública para o Módulo da Tabela de Símbolos.
 *
 * Este arquivo de cabeçalho define todas as estruturas de dados, tipos e
 * protótipos de funções para que outros módulos
 * possam interagir com a Tabela de Símbolos.
 *
 * --- O Algoritmo do Símbolo "Zumbi" ---
 *
 * Uma das técnicas mais importantes neste módulo é o gerenciamento de escopo
 * de parâmetros de funções através do estado "Zumbi".
 *
 * 1. O Problema: Quando uma função termina, seus parâmetros deixam de existir.
 * Se simplesmente os removermos da tabela, perdemos informações que podem ser
 * úteis para análises futuras ou para entender a estrutura completa do programa.
 *
 * 2. A Solução: Em vez de apagar o parâmetro, nós o "matamos", mudando seu
 * estado de "VIVO" para "ZUMBI_".
 *
 * 3. O Efeito:
 * - Um símbolo "ZUMBI_" é ignorado pela função de busca (`buscaLexPos`). Para
 * todos os efeitos, ele está fora de escopo e não pode ser encontrado ou
 * usado novamente.
 * - No entanto, ele permanece fisicamente na tabela. Isso preserva um
 * "histórico" completo de todas as declarações, o que é útil para depuração
 * e para entender a pilha de escopos em qualquer momento.
 *
 * A função `matarZumbis` é a responsável por aplicar este estado aos parâmetros
 * no final do escopo de uma função.
 */

#ifndef _TABELA_SIMBOLOS_
#define _TABELA_SIMBOLOS_

#include <stdbool.h>
#include <stdio.h>

//================================================================================
// 1. Definições de Tipos Enumerados (Enums)
//================================================================================

/** @brief Define o escopo de um símbolo (onde ele é válido). */
typedef enum {
    GLOBAL, ///< O símbolo é visível em todo o programa.
    LOCAL   ///< O símbolo é visível apenas dentro de um escopo específico (ex: uma função).
} ESCOPO;

/** @brief Define o tipo de dado de uma variável ou o retorno de uma função. */
typedef enum {
    INT_,     ///< Tipo inteiro.
    REAL_,    ///< Tipo ponto flutuante (real).
    CHAR_,    ///< Tipo caractere.
    BOOL_,    ///< Tipo booleano.
    NA_TIPO   ///< Indica "Não Aplicável", usado para procedimentos (void) ou outros casos.
} TIPO;

/** @brief Define a categoria de um identificador. */
typedef enum {
    VAR_GLOBAL, ///< Uma variável declarada no escopo global.
    VAR_LOCAL,  ///< Uma variável declarada no escopo local de uma função.
    PROC,       ///< Um procedimento ou função.
    PROC_PAR,   ///< Um parâmetro de um procedimento ou função.
    PROT_       ///< Um protótipo de função.
} IDCATEGORIA;

/**
 * @brief Define o status de atividade de um símbolo, crucial para o controle de escopo.
 * @see A explicação do "Algoritmo do Símbolo Zumbi" no cabeçalho deste arquivo.
 */
typedef enum {
    VIVO,   ///< O símbolo está ativo em seu escopo e pode ser encontrado em buscas.
    ZUMBI_  ///< O símbolo está fora de seu escopo (foi "morto"), tornando-o invisível para buscas, embora permaneça na tabela.
} ZUMBI;


//================================================================================
// 2. Definições de Estruturas de Dados (Structs)
//================================================================================

/**
 * @brief Estrutura principal que representa uma entrada na Tabela de Símbolos.
 *
 * Cada `tokenInfo` é como uma "ficha" que armazena todas as características
 * de um identificador (seu nome, tipo, escopo, etc.).
 */
typedef struct tokenInfo {
    char lexema[31];        ///< O nome do identificador.
    ESCOPO escopo;          ///< O escopo do identificador (GLOBAL ou LOCAL).
    TIPO tipo;              ///< O tipo de dado do identificador.
    IDCATEGORIA idcategoria;///< A categoria do identificador (variável, função, etc.).
    ZUMBI zumbi;            ///< O status de atividade do símbolo (VIVO ou ZUMBI_).
} TokenInfo;

/**
 * @brief A estrutura que representa a Tabela de Símbolos inteira.
 *
 * Funciona como uma pilha. O `topo` indica a próxima posição livre.
 * Novas entradas são adicionadas no topo, e a saída de escopo remove
 * entradas do topo.
 */
typedef struct tabela {
    int topo;                   ///< Ponteiro para o topo da pilha da tabela.
    TokenInfo tokensTab[1024];  ///< O array que armazena todas as entradas da tabela.
} Tabela;


//================================================================================
// 3. Interface Pública do Módulo
//================================================================================

//-------------------------------------------------
// Variáveis Globais Externas
//-------------------------------------------------

/** @brief Instância global da tabela, acessível por outros módulos. */
extern Tabela tabela;

//-------------------------------------------------
// Mapeamentos para Impressão (Depuração)
//-------------------------------------------------
/**
 * @brief Vetores externos de strings usados por `printarTabela` para exibir
 * os nomes dos enums em vez de seus valores numéricos, facilitando a depuração.
 */
extern char *T_escopo[];
extern char *T_tipo[];
extern char *T_IdCategoria[];
extern char *T_passagem[];
extern char *T_zumbi[];
extern char *T_array[];
extern char *T_ehConst[];


//-------------------------------------------------
// Protótipos das Funções
//-------------------------------------------------

/** @brief Exibe o conteúdo atual da tabela de símbolos no console. @param pos Posição a ser destacada. */
void printarTabela(int pos);

/** @brief Insere um novo símbolo (TokenInfo) na tabela. @param tokenInfo As informações do símbolo. */
void inserirNaTabela(TokenInfo tokenInfo);

/** @brief Remove o último símbolo inserido na tabela (operação de "pop"). */
void removerDaTabela();

/** @brief Limpa completamente a tabela, resetando-a ao estado inicial. */
void limparTabela();

/** @brief Reseta uma estrutura TokenInfo para um estado zerado. @param token Ponteiro para a estrutura a ser resetada. */
void resetTokenInfo(TokenInfo *token);

/** @brief Busca por declarações repetidas (redeclarações) de um símbolo. @param tokenInfo O símbolo a ser verificado. */
void buscaDeclRep(TokenInfo tokenInfo);

/** @brief Busca a posição da declaração mais recente de um símbolo. @param lexema O nome a ser buscado. @return O índice na tabela ou -1 se não encontrado. */
int buscaLexPos(char *lexema);

/** @brief Marca os parâmetros de uma função como "zumbis" ao sair do escopo. @param procPos Posição da função. */
void matarZumbis(int procPos);

/** @brief Remove todas as variáveis locais do escopo atual (topo da pilha). */
void retirarLocais();

/** @brief Busca por um símbolo e retorna sua estrutura de dados. Dispara erro se não encontrar. @param lexema O nome a ser buscado. @return A estrutura TokenInfo do símbolo. */
TokenInfo buscaDecl(char *lexema);


#endif // _TABELA_SIMBOLOS_