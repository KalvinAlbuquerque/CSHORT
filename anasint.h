/**
 * @file anasint.h
 * @brief Definição da Interface Pública para o Módulo de Análise Sintática.
 */
#ifndef _ANASINT_
#define _ANASINT_

#include <stdio.h>
#include "analex.h"
#include "tabela_simbolos.h"
#include "ansem.h"

//================================================================================
// 1. Variáveis Globais Externas
//================================================================================

extern TOKEN t;         ///< Token atual sendo processado.
extern FILE *fd;        ///< Ponteiro para o arquivo de código fonte.
extern int contLinha;   ///< Contador da linha atual no arquivo.
extern char TABS[200];   ///< String para controlar a indentação da árvore sintática.

//================================================================================
// 2. Assinaturas das Funções Públicas
//================================================================================

/**
 * @brief Ponto de entrada e regra inicial da gramática. Inicia a análise do programa.
 */
void Prog();

/**
 * @brief Analisa uma declaração de variável ou uma definição de função.
 */
void Decl_ou_Func();

/**
 * @brief Analisa o corpo de uma função, incluindo seus parâmetros e bloco de comandos.
 * @param procPos A posição da função na tabela de símbolos.
 * @return O número de parâmetros da função.
 */
int Func_body(int procPos); // <-- CORRIGIDO: de void para int

/**
 * @brief Analisa a lista de parâmetros de uma função.
 * @param func_pos A posição da função na tabela de símbolos.
 * @return O número de parâmetros encontrados.
 */
int Tipos_param(int func_pos); // <-- CORRIGIDO: de void para int

/**
 * @brief Analisa um comando (if, while, return, expressão, etc.).
 */
void Cmd();

/**
 * @brief Analisa uma expressão completa.
 * @return O tipo da expressão analisada.
 */
TipoExpressao Expr();


#endif // _ANASINT_