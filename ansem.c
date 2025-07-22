#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> // <-- NOVO: Necessário para funções com argumentos variáveis
#include "ansem.h"

// Pega as variáveis globais de linha e coluna do analisador léxico
extern int linha;
extern int coluna;

/**
 * @brief Reporta um erro semântico detalhado, incluindo a linha, e termina a execução.
 * @param formato Uma string de formato no estilo printf.
 * @param ... Argumentos variáveis para a string de formato.
 */
void erro_semantico(const char* formato, ...) {
    char buffer_msg[1024];
    va_list args;
    va_start(args, formato);
    vsnprintf(buffer_msg, sizeof(buffer_msg), formato, args);
    va_end(args);

    // Usa a função de erro global que já formata com a linha
    error(buffer_msg);
}


void checa_compatibilidade_atribuicao(TipoExpressao esq, TipoExpressao dir) {
    if (!esq.eh_lvalue) {
        erro_semantico("O lado esquerdo de uma atribuicao deve ser uma variavel. Expressao proxima a '%s' nao e valida.", dir.lexema);
    }

    // Regras de conversão implícita permitidas
    if ((esq.tipo == REAL_ && dir.tipo == INT_) ||
        (esq.tipo == INT_ && esq.tipo == REAL_) ||
        (esq.tipo == INT_ && dir.tipo == CHAR_) ||
        (esq.tipo == CHAR_ && dir.tipo == INT_) ||
        (esq.tipo == BOOL_ && dir.tipo == INT_) ||
        (esq.tipo == INT_ && dir.tipo == BOOL_)) {
        return; // Conversão permitida
    }

    if (esq.tipo != dir.tipo) {
        erro_semantico("Tipos incompativeis na atribuicao. Nao e possivel atribuir o tipo '%s' a variavel '%s' que e do tipo '%s'.",
                     T_tipo[dir.tipo], esq.lexema, T_tipo[esq.tipo]);
    }
}

TipoExpressao checa_compatibilidade_operador_aritmetico(TipoExpressao op1, TipoExpressao op2) {
    TipoExpressao resultado;
    resultado.eh_constante = op1.eh_constante && op2.eh_constante;
    resultado.eh_lvalue = 0;
    strcpy(resultado.lexema, "");

    if ((op1.tipo == INT_ || op1.tipo == CHAR_ || op1.tipo == REAL_) &&
        (op2.tipo == INT_ || op2.tipo == CHAR_ || op2.tipo == REAL_)) {
        if (op1.tipo == REAL_ || op2.tipo == REAL_) {
            resultado.tipo = REAL_;
        } else {
            resultado.tipo = INT_;
        }
        return resultado;
    }
    erro_semantico("Operadores aritmeticos exigem tipos numericos (int, char, real), mas foram recebidos '%s' e '%s'.",
                 T_tipo[op1.tipo], T_tipo[op2.tipo]);
    return resultado;
}

TipoExpressao checa_compatibilidade_operador_relacional(TipoExpressao op1, TipoExpressao op2) {
    TipoExpressao resultado;
    resultado.eh_constante = op1.eh_constante && op2.eh_constante;
    resultado.eh_lvalue = 0;
    resultado.tipo = BOOL_;
    strcpy(resultado.lexema, "");

    if ((op1.tipo == INT_ || op1.tipo == CHAR_ || op1.tipo == REAL_) &&
        (op2.tipo == INT_ || op2.tipo == CHAR_ || op2.tipo == REAL_)) {
        return resultado;
    }
    erro_semantico("Operadores relacionais exigem tipos numericos (int, char, real), mas foram recebidos '%s' e '%s'.",
                 T_tipo[op1.tipo], T_tipo[op2.tipo]);
    return resultado;
}

TipoExpressao checa_compatibilidade_operador_logico(TipoExpressao op1, TipoExpressao op2) {
    TipoExpressao resultado;
    resultado.eh_constante = op1.eh_constante && op2.eh_constante;
    resultado.eh_lvalue = 0;
    resultado.tipo = BOOL_;
    strcpy(resultado.lexema, "");

    if ((op1.tipo == BOOL_ || op1.tipo == INT_) && (op2.tipo == BOOL_ || op2.tipo == INT_)) {
        return resultado;
    }
    erro_semantico("Operadores logicos (&&, ||) exigem tipos compativeis com booleano, mas foram recebidos '%s' e '%s'.",
                 T_tipo[op1.tipo], T_tipo[op2.tipo]);
    return resultado;
}

void checa_condicao_booleana(char* comando, TipoExpressao expr) {
    if (expr.tipo != BOOL_ && expr.tipo != INT_) {
        erro_semantico("A expressao condicional em um comando '%s' deve ser do tipo booleano, mas foi recebido o tipo '%s'.",
                     comando, T_tipo[expr.tipo]);
    }
}

void checa_retorno_funcao(TIPO tipo_retorno_declarado, char* nome_funcao, TipoExpressao expr_retornada, int tem_expr) {
    if (tipo_retorno_declarado == NA_TIPO) {
        if (tem_expr) {
            erro_semantico("A funcao '%s' e do tipo 'void' e nao pode retornar um valor.", nome_funcao);
        }
    } else {
        if (!tem_expr) {
            erro_semantico("A funcao '%s' deve retornar um valor do tipo '%s', mas nenhum valor foi retornado.",
                         nome_funcao, T_tipo[tipo_retorno_declarado]);
        }
        // Simplificando a checagem de compatibilidade
        if (tipo_retorno_declarado != expr_retornada.tipo) {
            if (!((tipo_retorno_declarado == INT_ && (expr_retornada.tipo == CHAR_ || expr_retornada.tipo == BOOL_)) ||
                  (tipo_retorno_declarado == REAL_ && expr_retornada.tipo == INT_) ||
                  (tipo_retorno_declarado == CHAR_ && expr_retornada.tipo == INT_) ||
                  (tipo_retorno_declarado == BOOL_ && expr_retornada.tipo == INT_))) {
                 erro_semantico("Tipo de retorno incompativel. A funcao '%s' espera '%s', mas a expressao retornada e do tipo '%s'.",
                              nome_funcao, T_tipo[tipo_retorno_declarado], T_tipo[expr_retornada.tipo]);
            }
        }
    }
}