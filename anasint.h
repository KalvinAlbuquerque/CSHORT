// Anasint.h
#ifndef ANASINT_H
#define ANASINT_H

#include <stdio.h>
#include "analex.h" // Inclui o seu analisador léxico do Cshort

// --- Variáveis Globais ---
extern TOKEN t;       // Token atual, lido pelo Analex
extern FILE *fd;      // Ponteiro para o arquivo de código fonte
extern char TABS[200]; // Declara que a variável TABS existe em outro arquivo

// --- Funções do Analisador Sintático (baseadas na gramática) ---

// Função inicial que começa a análise
void Prog();

// Funções para declarações
void Decl();
void Decl_var();
int Tipo(); // Retorna 1 se encontrou um tipo, 0 caso contrário
void Tipos_param();

// Função para análise de funções
void Func();

// Funções para comandos
void Cmd();
void Cmd_if();
void Cmd_while();
void Cmd_for();
void Cmd_return();
void Atrib();

// Funções para análise de expressões
void Expr();
void Expr_simp();
void Termo();
void Fator();

#endif