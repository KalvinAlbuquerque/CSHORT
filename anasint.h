#ifndef ANASINT_H
#define ANASINT_H

#include <stdio.h>
#include "analex.h"
#include "ansem.h" // <-- ADICIONADO: Garante que TipoExpressao seja conhecido

//================================================================================
// 1. Variáveis Globais
//================================================================================
extern FILE *fd;
extern TOKEN t;
extern char TABS[200];


//================================================================================
// 2. Protótipos das Funções de Análise
//================================================================================
void aumenta_ident();
void diminui_ident();
void print_folha(TOKEN tk);
void consome(int categoria_esperada, int codigo_esperado);

void Prog();
void Decl_ou_Func();
void Func_body(int procPos);
void Decl();
void Decl_var_body();
void Decl_var();
int Tipo();
void Tipos_param(int func_pos);
void Cmd();

// Protótipos das funções de expressão corrigidos
TipoExpressao Expr();
TipoExpressao Expr_atrib();
TipoExpressao Expr_ou();
TipoExpressao Expr_e();
TipoExpressao Expr_relacional();
TipoExpressao Expr_aditiva();
TipoExpressao Expr_multiplicativa();
TipoExpressao Fator();

#endif // ANASINT_H