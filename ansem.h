#ifndef ANSEM_H
#define ANSEM_H

#include "tabela_simbolos.h"
#include "analex.h"

// Estrutura para representar o tipo de um resultado de expressão
typedef struct {
    TIPO tipo;
    int eh_constante;
    int eh_lvalue;
    char lexema[31]; // <-- NOVO: Armazena o nome do ID, se aplicável
} TipoExpressao;


// Funções de Checagem Semântica
void erro_semantico(const char* formato, ...); // <-- Permite formatação
void checa_compatibilidade_atribuicao(TipoExpressao esq, TipoExpressao dir);
TipoExpressao checa_compatibilidade_operador_aritmetico(TipoExpressao op1, TipoExpressao op2);
TipoExpressao checa_compatibilidade_operador_relacional(TipoExpressao op1, TipoExpressao op2);
TipoExpressao checa_compatibilidade_operador_logico(TipoExpressao op1, TipoExpressao op2);
void checa_condicao_booleana(char* comando, TipoExpressao expr);
void checa_retorno_funcao(TIPO tipo_retorno_declarado, char* nome_funcao, TipoExpressao expr_retornada, int tem_expr);

#endif