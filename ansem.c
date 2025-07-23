/**
 * @file ansem.c
 * @author Seu Nome/Nome do Projeto
 * @date 22/07/2025
 *
 * @brief Módulo de Análise Semântica para o compilador C-Short.
 *
 * @details
 * Este arquivo contém as funções responsáveis por validar as regras semânticas
 * da linguagem. A análise semântica ocorre em conjunto com a análise sintática
 * e é crucial para garantir que o programa, embora sintaticamente correto,
 * também faça sentido lógico.
 *
 * As principais responsabilidades deste módulo incluem:
 * 1.  **Checagem de Tipos**: Verificar se os operandos de uma expressão são
 * compatíveis com o operador (ex: não se pode somar um booleano com um array).
 * 2.  **Validação de Atribuições**: Garantir que o tipo de uma expressão sendo
 * atribuída é compatível com o tipo da variável que a recebe.
 * 3.  **Verificação de L-values**: Assegurar que o lado esquerdo de uma atribuição
 * seja uma expressão modificável (uma variável).
 * 4.  **Validação de Retornos**: Checar se o valor retornado por uma função é
 * compatível com o tipo de retorno declarado para ela.
 * 5.  **Reporte de Erros**: Fornecer mensagens de erro claras e informativas
 * quando uma regra semântica é violada, indicando a linha do erro.
 *
 * As funções deste módulo são chamadas pelo analisador sintático (`anasint.c`)
 * nos pontos apropriados da gramática para realizar essas validações.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> // Necessário para funções com argumentos variáveis
#include "ansem.h"

// Disponibiliza as variáveis globais de linha e coluna do analisador léxico para este arquivo.
extern int contLinha; // Assume-se que 'contLinha' é o nome da variável no analex.

/**
 * @brief Reporta um erro semântico detalhado e termina a execução do compilador.
 *
 * Esta função utiliza argumentos variáveis para formatar uma mensagem de erro
 * personalizada, similar à função `printf`. Ela adiciona automaticamente o número
 * da linha onde o erro ocorreu, facilitando a depuração do código-fonte.
 *
 * @param formato Uma string de formato no estilo printf.
 * @param ... Argumentos variáveis para a string de formato.
 */
void erro_semantico(const char* formato, ...) {
    char buffer_msg[1024];
    va_list args;

    // Inicia a lista de argumentos variáveis
    va_start(args, formato);
    // Formata a mensagem de erro no buffer
    vsnprintf(buffer_msg, sizeof(buffer_msg), formato, args);
    // Finaliza o uso da lista de argumentos
    va_end(args);

    // Imprime a mensagem de erro final, incluindo o número da linha
    fprintf(stderr, "Erro Semantico na linha %d: %s\n", contLinha, buffer_msg);
    exit(1); // Encerra o compilador
}

/**
 * @brief Verifica a compatibilidade de tipos em uma operação de atribuição.
 *
 * Valida se o tipo da expressão do lado direito (`dir`) pode ser atribuído
 * a uma variável do tipo do lado esquerdo (`esq`). Permite algumas conversões
 * implícitas, como atribuir um `int` a um `real`.
 *
 * @param esq A expressão do lado esquerdo (deve ser um L-value).
 * @param dir A expressão do lado direito.
 */
void checa_compatibilidade_atribuicao(TipoExpressao esq, TipoExpressao dir) {
    // A primeira verificação é garantir que o lado esquerdo é uma variável.
    if (!esq.eh_lvalue) {
        erro_semantico("O lado esquerdo de uma atribuicao deve ser uma variavel. Expressao proxima a '%s' nao e valida.", dir.lexema);
    }

    // Se os tipos já são idênticos, a atribuição é válida.
    if (esq.tipo == dir.tipo) {
        return;
    }

    // Regras de conversão implícita permitidas (ex: int -> real, char -> int).
    if ((esq.tipo == REAL_ && dir.tipo == INT_) ||
        (esq.tipo == INT_ && dir.tipo == CHAR_) ||
        (esq.tipo == CHAR_ && dir.tipo == INT_) ||
        (esq.tipo == BOOL_ && dir.tipo == INT_) ||
        (esq.tipo == INT_ && dir.tipo == BOOL_)) {
        // A geração de código para a conversão (CAST) deveria ser feita aqui, se necessário.
        return; // Conversão permitida.
    }

    // Se nenhuma das condições acima for satisfeita, os tipos são incompatíveis.
    erro_semantico("Tipos incompativeis na atribuicao. Nao e possivel atribuir o tipo '%s' a variavel '%s' que e do tipo '%s'.",
                     T_tipo[dir.tipo], esq.lexema, T_tipo[esq.tipo]);
}

/**
 * @brief Verifica a compatibilidade de tipos para operadores aritméticos (+, -, *, /).
 *
 * Garante que ambos os operandos sejam de tipos numéricos (int, char, real).
 * O tipo do resultado será `REAL` se um dos operandos for `REAL`, caso contrário, será `INT`.
 *
 * @param op1 O primeiro operando.
 * @param op2 O segundo operando.
 * @return Uma estrutura `TipoExpressao` descrevendo o tipo do resultado da operação.
 */
TipoExpressao checa_compatibilidade_operador_aritmetico(TipoExpressao op1, TipoExpressao op2) {
    TipoExpressao resultado;
    resultado.eh_lvalue = 0; // O resultado de uma operação nunca é um L-value.
    strcpy(resultado.lexema, "");

    // Verifica se ambos os operandos são numéricos.
    if ((op1.tipo == INT_ || op1.tipo == CHAR_ || op1.tipo == REAL_) &&
        (op2.tipo == INT_ || op2.tipo == CHAR_ || op2.tipo == REAL_)) {

        // Regra de promoção: se um operando for REAL, o resultado é REAL.
        if (op1.tipo == REAL_ || op2.tipo == REAL_) {
            resultado.tipo = REAL_;
        } else {
            resultado.tipo = INT_;
        }
        return resultado;
    }

    // Se os tipos não forem numéricos, reporta um erro.
    erro_semantico("Operadores aritmeticos exigem tipos numericos (int, char, real), mas foram recebidos '%s' e '%s'.",
                   T_tipo[op1.tipo], T_tipo[op2.tipo]);
    return resultado; // Esta linha nunca é alcançada devido ao exit() em erro_semantico.
}

/**
 * @brief Verifica a compatibilidade de tipos para operadores relacionais (==, !=, <, >, <=, >=).
 *
 * Garante que ambos os operandos sejam de tipos numéricos. O resultado de uma
 * operação relacional é sempre do tipo `BOOL`.
 *
 * @param op1 O primeiro operando.
 * @param op2 O segundo operando.
 * @return Uma estrutura `TipoExpressao` descrevendo o tipo do resultado (sempre `BOOL`).
 */
TipoExpressao checa_compatibilidade_operador_relacional(TipoExpressao op1, TipoExpressao op2) {
    TipoExpressao resultado;
    resultado.eh_lvalue = 0;
    resultado.tipo = BOOL_; // O resultado de uma comparação é sempre booleano.
    strcpy(resultado.lexema, "");

    // Garante que a comparação seja feita entre tipos numéricos.
    if ((op1.tipo == INT_ || op1.tipo == CHAR_ || op1.tipo == REAL_) &&
        (op2.tipo == INT_ || op2.tipo == CHAR_ || op2.tipo == REAL_)) {
        return resultado;
    }

    erro_semantico("Operadores relacionais exigem tipos numericos (int, char, real), mas foram recebidos '%s' e '%s'.",
                   T_tipo[op1.tipo], T_tipo[op2.tipo]);
    return resultado;
}

/**
 * @brief Verifica a compatibilidade de tipos para operadores lógicos (&&, ||).
 *
 * Permite que operandos sejam do tipo `BOOL` ou `INT` (onde 0 é falso e não-zero é verdadeiro).
 * O resultado de uma operação lógica é sempre do tipo `BOOL`.
 *
 * @param op1 O primeiro operando.
 * @param op2 O segundo operando.
 * @return Uma estrutura `TipoExpressao` descrevendo o tipo do resultado (sempre `BOOL`).
 */
TipoExpressao checa_compatibilidade_operador_logico(TipoExpressao op1, TipoExpressao op2) {
    TipoExpressao resultado;
    resultado.eh_lvalue = 0;
    resultado.tipo = BOOL_;
    strcpy(resultado.lexema, "");

    // Permite operações lógicas entre tipos BOOL ou INT.
    if ((op1.tipo == BOOL_ || op1.tipo == INT_) && (op2.tipo == BOOL_ || op2.tipo == INT_)) {
        return resultado;
    }

    erro_semantico("Operadores logicos (&&, ||) exigem tipos compativeis com booleano, mas foram recebidos '%s' e '%s'.",
                   T_tipo[op1.tipo], T_tipo[op2.tipo]);
    return resultado;
}

/**
 * @brief Verifica se a expressão em uma condição (if, while) é do tipo booleano.
 *
 * @param comando O nome do comando que contém a condição (ex: "if", "while").
 * @param expr A expressão da condição a ser verificada.
 */
void checa_condicao_booleana(char* comando, TipoExpressao expr) {
    // Uma condição pode ser do tipo BOOL ou INT.
    if (expr.tipo != BOOL_ && expr.tipo != INT_) {
        erro_semantico("A expressao condicional em um comando '%s' deve ser do tipo booleano ou inteiro, mas foi recebido o tipo '%s'.",
                       comando, T_tipo[expr.tipo]);
    }
}

/**
 * @brief Verifica se o tipo de retorno de uma função está correto.
 *
 * Compara o tipo declarado da função com o tipo da expressão no comando `return`.
 * Também verifica se uma função `void` não retorna valor e se uma função não-`void`
 * sempre retorna um valor.
 *
 * @param tipo_retorno_declarado O tipo de retorno esperado, conforme a declaração da função.
 * @param nome_funcao O nome da função, para mensagens de erro.
 * @param expr_retornada A expressão que está sendo retornada.
 * @param tem_expr Flag que indica se o comando `return` possui uma expressão.
 */
void checa_retorno_funcao(TIPO tipo_retorno_declarado, char* nome_funcao, TipoExpressao expr_retornada, int tem_expr) {
    // Caso 1: A função é 'void'.
    if (tipo_retorno_declarado == NA_TIPO) {
        if (tem_expr) { // Se for void, não pode ter expressão de retorno.
            erro_semantico("A funcao '%s' e do tipo 'void' e nao pode retornar um valor.", nome_funcao);
        }
    }
    // Caso 2: A função não é 'void'.
    else {
        if (!tem_expr) { // Se não for void, DEVE ter uma expressão de retorno.
            erro_semantico("A funcao '%s' deve retornar um valor do tipo '%s', mas nenhum valor foi retornado.",
                             nome_funcao, T_tipo[tipo_retorno_declarado]);
        }

        // Verifica a compatibilidade de tipos entre o retorno declarado e o retornado.
        if (tipo_retorno_declarado != expr_retornada.tipo) {
            // Verifica se é um caso de conversão implícita permitida.
            if (!((tipo_retorno_declarado == INT_ && (expr_retornada.tipo == CHAR_ || expr_retornada.tipo == BOOL_)) ||
                  (tipo_retorno_declarado == REAL_ && expr_retornada.tipo == INT_) ||
                  (tipo_retorno_declarado == CHAR_ && expr_retornada.tipo == INT_) ||
                  (tipo_retorno_declarado == BOOL_ && expr_retornada.tipo == INT_))) {
                // Se não for um tipo idêntico nem uma conversão permitida, gera erro.
                erro_semantico("Tipo de retorno incompativel. A funcao '%s' espera '%s', mas a expressao retornada e do tipo '%s'.",
                               nome_funcao, T_tipo[tipo_retorno_declarado], T_tipo[expr_retornada.tipo]);
            }
        }
    }
}