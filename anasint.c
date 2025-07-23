/**
 * @file anasint.c
 * @author Seu Nome/Nome do Projeto
 * @date 22/07/2025
 *
 * @brief Analisador Sintático e Semântico (Parser) para a linguagem C-Short.
 *
 * @details
 * Este arquivo implementa um Analisador Sintático de Descida Recursiva. Cada função
 * representa uma regra da gramática da linguagem, validando a estrutura do código-fonte
 * e, ao mesmo tempo, realizando a análise semântica e a geração de código intermediário.
 *
 * == ARQUITETURA E FLUXO ==
 * 1.  **Ponto de Entrada**: A função `Prog()` é o ponto de partida, representando a regra
 * inicial da gramática.
 * 2.  **Descida Recursiva**: As funções se chamam recursivamente para analisar a estrutura
 * do código. Por exemplo, `Prog()` chama `Decl_ou_Func()`, que por sua vez pode
 * chamar `Func_body()`, que chama `Cmd()`, e assim por diante.
 * 3.  **Consumo de Tokens**: A função `consome()` avança na análise, verificando se o
 * token atual é o esperado pela gramática e solicitando o próximo token ao
 * Analisador Léxico (`analex`).
 * 4.  **Análise Semântica**: Durante a análise, são feitas chamadas para funções do
 * módulo de análise semântica (`ansem.h`) para verificar a compatibilidade de
 * tipos, declaração de variáveis, etc. (ex: `checa_compatibilidade_atribuicao`).
 * 5.  **Geração de Código**: À medida que as regras gramaticais são validadas, são
 * feitas chamadas ao módulo gerador de código (`gerador_codigo.h`) para emitir
 * as instruções da máquina de pilha (ex: `gerar_codigo("PUSH ...")`).
 * 6.  **Tabela de Símbolos**: A tabela de símbolos (`tabela_simbolos.h`) é constantemente
 * consultada e atualizada para gerenciar informações sobre variáveis e funções
 * (escopo, tipo, offset de memória).
 *
 * == INTERAÇÃO COM OUTROS MÓDULOS ==
 * - **analex.h**: Fornece a entrada de tokens para o analisador sintático.
 * - **tabela_simbolos.h**: Armazena e recupera informações sobre identificadores.
 * - **ansem.h**: Contém as rotinas para validação das regras semânticas.
 * - **gerador_codigo.h**: Responsável por escrever o código de máquina de pilha no
 * arquivo de saída.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anasint.h"
#include "analex.h"
#include "tabela_simbolos.h"
#include "ansem.h"
#include "gerador_codigo.h"

// --- Variáveis Globais ---

/// @brief Estrutura temporária para armazenar informações do token sendo processado.
TokenInfo tokenInfo;
/// @brief Armazena o nome da função que está sendo analisada no momento.
char nome_funcao_atual[31];
/// @brief Armazena o tipo de retorno da função atual, para checagem semântica do 'return'.
TIPO tipo_retorno_funcao_atual;

/// @brief Contador para o deslocamento (offset) de variáveis globais na memória.
int offset_global = 0;
/// @brief Contador para o deslocamento (offset) de variáveis locais na memória de uma função.
int offset_local = 0;
/// @brief Contador para o deslocamento (offset) de parâmetros de uma função.
int offset_param = 0;

// --- Protótipos de Funções Internas da Análise de Descida Recursiva ---
void Prog();
void Decl_ou_Func();
int Func_body(int procPos);
void Decl_var_body();
void Decl();
void Decl_var();
int Tipo();
int Tipos_param(int func_pos);
void Cmd();
TipoExpressao Expr();
TipoExpressao Expr_atrib();
TipoExpressao Expr_ou();
TipoExpressao Expr_e();
TipoExpressao Expr_relacional();
TipoExpressao Expr_aditiva();
TipoExpressao Expr_multiplicativa();
TipoExpressao Fator();

// --- Funções Auxiliares ---

/**
 * @brief Aumenta a indentação para a impressão da árvore de análise sintática.
 */
void aumenta_ident() {
    if (strlen(TABS) < sizeof(TABS) - 3) strcat(TABS, "  ");
}

/**
 * @brief Diminui a indentação para a impressão da árvore de análise sintática.
 */
void diminui_ident() {
    if (strlen(TABS) >= 2) TABS[strlen(TABS) - 2] = '\0';
}

/**
 * @brief Imprime um nó folha da árvore de análise (um token).
 * @param tk O token a ser impresso.
 */
void print_folha(TOKEN tk) {
    printf("%s%s (%s)\n", TABS, tk.lexema, tk.cat == ID ? "ID" : "Palavra Reservada");
}

/**
 * @brief Verifica se o token atual é o esperado e avança para o próximo.
 * Se o token atual não for o esperado, a função encerra o programa com erro.
 * @param categoria_esperada A categoria do token que a gramática espera.
 * @param codigo_esperado O código do token (para palavras reservadas, sinais, etc.).
 * Se for 0, qualquer código dentro da categoria é aceito.
 */
void consome(int categoria_esperada, int codigo_esperado) {
    if (t.cat == categoria_esperada && (codigo_esperado == 0 || t.codigo == codigo_esperado)) {
        t = Analex(fd); // Avança para o próximo token
    } else {
        // Erro sintático: o token encontrado não era o esperado.
        printf("Erro na linha %d: Token inesperado.\n", contLinha);
        printf("Esperado: Categoria %d, Codigo %d\n", categoria_esperada, codigo_esperado);
        printf("Encontrado: Categoria %d, Codigo %d, Lexema '%s'\n", t.cat, t.codigo, t.lexema);
        exit(1);
    }
}

/**
 * @brief Verifica se o token atual é um tipo de dado válido (int, real, bool, char).
 * Se for, atualiza a variável global `tokenInfo` com o tipo encontrado.
 * @return Retorna 1 se o token for um tipo, 0 caso contrário.
 */
int Tipo() {
    if (t.cat == PALAVRA_RESERVADA) {
        if (t.codigo == PR_INTCON || t.codigo == PR_REALCON || t.codigo == PR_BOOL || t.codigo == PR_CHARCON) {
            switch(t.codigo) {
                case PR_INTCON: tokenInfo.tipo = INT_; break;
                case PR_REALCON: tokenInfo.tipo = REAL_; break;
                case PR_CHARCON: tokenInfo.tipo = CHAR_; break;
                case PR_BOOL: tokenInfo.tipo = BOOL_; break;
                default: tokenInfo.tipo = NA_TIPO; break;
            }
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Gera uma instrução 'LOAD' se a expressão for uma variável (L-value).
 * Esta função adia a geração do código de carregamento para o momento em que o VALOR
 * da variável é de fato necessário na pilha, evitando carregar o valor de uma
 * variável que está do lado esquerdo de uma atribuição.
 * @param expr A estrutura da expressão a ser verificada.
 */
void gerar_load_se_necessario(TipoExpressao expr) {
    if (expr.eh_lvalue) {
        TokenInfo id_info = buscaDecl(expr.lexema);
        // Gera 'LOAD escopo, offset'. Escopo 0 para global, 1 para local.
        gerar_codigo("LOAD %d, %d", id_info.escopo == GLOBAL ? 0 : 1, id_info.offset);
    }
}

// --- Funções de Análise Sintática (Descida Recursiva) ---

/**
 * @brief Ponto de entrada da análise. Regra: <Prog> -> { <Decl_ou_Func> }
 */
void Prog() {
    printf("%s<Prog>\n", TABS);
    aumenta_ident();
    t = Analex(fd); // Lê o primeiro token do arquivo

    gerar_codigo("INIP"); // Gera o código de inicialização do programa

    // Loop principal que processa todas as declarações de variáveis globais e funções
    while (Tipo() || (t.cat == PALAVRA_RESERVADA && t.codigo == PR_VOID)) {
        Decl_ou_Func();
    }
    
    // Verificação semântica: garante que a função 'main' foi declarada.
    if (buscaLexPos("main") == -1) {
        erro_semantico("Funcao 'main' nao declarada no programa.");
    }

    gerar_codigo("CALL main"); // Gera a chamada para a função principal
    gerar_codigo("HALT");     // Gera a instrução para terminar o programa
    
    limparTabela();
    diminui_ident();
    printf("%s</Prog>\n", TABS);
}

/**
 * @brief Processa uma declaração de variável global ou uma definição de função.
 * Regra: <Decl_ou_Func> -> <Tipo> ID ( <Decl_Var_body> | <Func_body> )
 */
void Decl_ou_Func() {
    printf("%s<Decl_ou_Func>\n", TABS);
    aumenta_ident();
    
    int tipo_atual;
    if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_VOID) {
        tipo_atual = NA_TIPO;
    } else {
        tipo_atual = tokenInfo.tipo;
    }

    print_folha(t);
    consome(t.cat, t.codigo); // Consome o tipo (int, void, etc.)
    
    // Armazena informações do identificador (nome da var/função)
    strcpy(tokenInfo.lexema, t.lexema);
    strcpy(nome_funcao_atual, t.lexema);
    tokenInfo.tipo = tipo_atual;

    print_folha(t);
    consome(ID, 0); // Consome o ID

    // Se o próximo token for '(', trata-se de uma função.
    if (t.cat == SN && t.codigo == ABRE_PARENTESES) {
        // Gera o rótulo (LABEL) para o início do código da função
        if (strcmp(tokenInfo.lexema, "main") == 0) {
            gerar_codigo("LABEL main");
        } else {
            gerar_codigo("LABEL %s", tokenInfo.lexema);
        }
        tokenInfo.idcategoria = PROC;
        tipo_retorno_funcao_atual = tokenInfo.tipo;
        
        int func_pos = tabela.topo;
        inserirNaTabela(tokenInfo);
        
        int num_params = Func_body(func_pos);
        
        // Limpeza semântica e da tabela de símbolos
        matarZumbis(func_pos);
        retirarLocais();
        
        // Gera a instrução de retorno da função
        gerar_codigo("RET 0, %d", num_params);
    } 
    // Caso contrário, é uma declaração de variável global.
    else {
        tokenInfo.idcategoria = VAR_GLOBAL;
        tokenInfo.escopo = GLOBAL;
        tokenInfo.offset = offset_global++;
        inserirNaTabela(tokenInfo);
        Decl_var_body();
        gerar_codigo("AMEM 1"); // Aloca memória para a variável global
    }
    diminui_ident();
    printf("%s</Decl_ou_Func>\n", TABS);
}

/**
 * @brief Processa o corpo de uma função (parâmetros e bloco de comandos).
 * Regra: <Func_body> -> '(' [ <Tipos_param> ] ')' ( ';' | '{' { <Decl> } { <Cmd> } '}' )
 * @param procPos Posição da função na tabela de símbolos.
 * @return O número de parâmetros da função.
 */
int Func_body(int procPos) {
    printf("%s<Func_body>\n", TABS);
    aumenta_ident();
    
    print_folha(t);
    consome(SN, ABRE_PARENTESES);
    
    // Reseta os contadores de offset para o novo escopo da função
    offset_local = 0;
    offset_param = 0;

    int num_params = 0;
    if (t.cat != SN || t.codigo != FECHA_PARENTESES) {
        num_params = Tipos_param(procPos); // Processa a lista de parâmetros
    }

    print_folha(t);
    consome(SN, FECHA_PARENTESES);

    // INIPR é gerado para funções, mas não para a 'main'
    if (strcmp(nome_funcao_atual, "main") != 0) {
        gerar_codigo("INIPR 1");
    }

    if (t.cat == SN && t.codigo == PONTO_VIRGULA) { // Protótipo de função
        print_folha(t);
        consome(SN, PONTO_VIRGULA);
    } else { // Definição de função
        print_folha(t);
        consome(SN, ABRE_CHAVES);
        
        int contador_locais = 0;
        // Processa as declarações de variáveis locais
        while (Tipo()) {
            tokenInfo.idcategoria = VAR_LOCAL;
            tokenInfo.escopo = LOCAL;
            Decl();
            contador_locais++;
        }
        
        if (contador_locais > 0) {
            gerar_codigo("AMEM %d", contador_locais); // Aloca memória para todas as vars locais
        }
        
        // Processa os comandos dentro da função
        while (!(t.cat == SN && t.codigo == FECHA_CHAVES)) {
            Cmd();
        }
        
        if (contador_locais > 0) {
            gerar_codigo("DMEM %d", contador_locais); // Desaloca a memória das vars locais
        }
        
        print_folha(t);
        consome(SN, FECHA_CHAVES);
    }
    diminui_ident();
    printf("%s</Func_body>\n", TABS);
    return num_params;
}

/**
 * @brief Processa o final de uma declaração de variável.
 */
void Decl_var_body() {
    if (t.cat == SN && t.codigo == PONTO_VIRGULA) {
        print_folha(t);
        consome(SN, PONTO_VIRGULA);
    }
}

/**
 * @brief Processa uma linha de declaração de variáveis.
 * Regra: <Decl> -> <Tipo> <Decl_var> { ',' <Decl_var> } ';'
 */
void Decl() {
    printf("%s<Decl>\n", TABS);
    aumenta_ident();
    
    if (Tipo()) {
        print_folha(t);
        consome(PALAVRA_RESERVADA, t.codigo); // Consome o tipo
        Decl_var();
        while (t.cat == SN && t.codigo == VIRGULA) {
            print_folha(t);
            consome(SN, VIRGULA);
            Decl_var();
        }
        print_folha(t);
        consome(SN, PONTO_VIRGULA);
    }
    
    diminui_ident();
    printf("%s</Decl>\n", TABS);
}

/**
 * @brief Processa um único identificador em uma declaração.
 * Regra: <Decl_var> -> ID [ '[' CT_INT ']' ]
 */
void Decl_var() {
    printf("%s<Decl_var>\n", TABS);
    aumenta_ident();
    
    strcpy(tokenInfo.lexema, t.lexema);
    print_folha(t);
    consome(ID, 0);

    // Define o offset da variável com base no seu escopo
    if (tokenInfo.idcategoria == VAR_LOCAL) {
        tokenInfo.offset = offset_local++;
    } else if (tokenInfo.idcategoria == PROC_PAR) {
        tokenInfo.offset = offset_param++;
    }

    // Processa declaração de arrays (ainda sem geração de código completa)
    if (t.cat == SN && t.codigo == ABRE_COLCHETES) {
        print_folha(t); consome(SN, ABRE_COLCHETES);
        print_folha(t); consome(CT_INT, 0);
        print_folha(t); consome(SN, FECHA_COLCHETES);
    }
    
    inserirNaTabela(tokenInfo); // Insere a variável na tabela de símbolos
    
    diminui_ident();
    printf("%s</Decl_var>\n", TABS);
}

/**
 * @brief Processa a lista de parâmetros de uma função.
 * @param func_pos Posição da função na tabela de símbolos.
 * @return O número de parâmetros encontrados.
 */
int Tipos_param(int func_pos) {
    printf("%s<Tipos_param>\n", TABS);
    aumenta_ident();
    
    int count = 0;
    if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_VOID) {
        print_folha(t);
        consome(PALAVRA_RESERVADA, PR_VOID);
    } else {
        while (Tipo()) {
            count++;
            tokenInfo.idcategoria = PROC_PAR;
            tokenInfo.escopo = LOCAL;
            print_folha(t);
            consome(PALAVRA_RESERVADA, t.codigo);
            Decl_var();
            if (t.cat == SN && t.codigo == VIRGULA) {
                print_folha(t);
                consome(SN, VIRGULA);
            } else {
                break;
            }
        }
    }
    
    diminui_ident();
    printf("%s</Tipos_param>\n", TABS);
    return count;
}

/**
 * @brief Processa um comando (if, while, return, atribuição, etc.).
 * Regra: <Cmd> -> <Cmd_if> | <Cmd_while> | <Cmd_return> | '{' { <Cmd> } '}' | <Expr> ';'
 */
void Cmd() {
    printf("%s<Cmd>\n", TABS);
    aumenta_ident();
    
    if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_IF) {
        // Bloco IF-ELSE
        print_folha(t); consome(PALAVRA_RESERVADA, PR_IF);
        print_folha(t); consome(SN, ABRE_PARENTESES);
        TipoExpressao expr = Expr();
        gerar_load_se_necessario(expr);
        print_folha(t); consome(SN, FECHA_PARENTESES);
        
        char rotulo_falso[10];
        novo_rotulo(rotulo_falso);
        gerar_codigo("GOFALSE %s", rotulo_falso); // Se a condição for falsa, pula o bloco 'if'

        Cmd();

        if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_ELSE) {
            char rotulo_fim[10];
            novo_rotulo(rotulo_fim);
            gerar_codigo("GOTO %s", rotulo_fim); // Pula o bloco 'else'
            gerar_codigo("LABEL %s", rotulo_falso); // Ponto de pulo para a condição falsa

            print_folha(t); consome(PALAVRA_RESERVADA, PR_ELSE);
            Cmd();
            gerar_codigo("LABEL %s", rotulo_fim); // Ponto de pulo para o final do 'if-else'
        } else {
            gerar_codigo("LABEL %s", rotulo_falso); // Ponto de pulo se não houver 'else'
        }
    } else if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_WHILE) {
        // Bloco WHILE
        char rotulo_inicio[10], rotulo_fim[10];
        novo_rotulo(rotulo_inicio);
        novo_rotulo(rotulo_fim);

        gerar_codigo("LABEL %s", rotulo_inicio); // Rótulo para o início do loop
        
        print_folha(t); consome(PALAVRA_RESERVADA, PR_WHILE);
        print_folha(t); consome(SN, ABRE_PARENTESES);
        TipoExpressao expr = Expr();
        gerar_load_se_necessario(expr);
        gerar_codigo("GOFALSE %s", rotulo_fim); // Se a condição for falsa, sai do loop
        print_folha(t); consome(SN, FECHA_PARENTESES);
        Cmd();
        gerar_codigo("GOTO %s", rotulo_inicio); // Volta para o início do loop
        gerar_codigo("LABEL %s", rotulo_fim);   // Rótulo para o fim do loop

    } else if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_RETURN) {
        // Comando RETURN
        print_folha(t); consome(PALAVRA_RESERVADA, PR_RETURN);
        if (t.cat != SN || t.codigo != PONTO_VIRGULA) {
            TipoExpressao expr = Expr();
            gerar_load_se_necessario(expr); // Carrega o valor de retorno para a pilha
        } else {
             // Caso 'return;' sem valor (ex: na main), empilha 0 por padrão.
             gerar_codigo("PUSH 0");
        }
        print_folha(t); consome(SN, PONTO_VIRGULA);
    } else if (t.cat == SN && t.codigo == ABRE_CHAVES) {
        // Bloco de comandos
        print_folha(t); consome(SN, ABRE_CHAVES);
        while (t.cat != SN || t.codigo != FECHA_CHAVES) {
            Cmd();
        }
        print_folha(t); consome(SN, FECHA_CHAVES);
    } else {
        // Expressão seguida de ponto e vírgula (ex: uma atribuição ou chamada de função)
        Expr();
        print_folha(t); consome(SN, PONTO_VIRGULA);
    }
    
    diminui_ident();
    printf("%s</Cmd>\n", TABS);
}

/**
 * @brief Ponto de partida para análise de qualquer expressão.
 */
TipoExpressao Expr() {
    printf("%s<Expr>\n", TABS);
    aumenta_ident();
    TipoExpressao tipo = Expr_atrib();
    diminui_ident();
    printf("%s</Expr>\n", TABS);
    return tipo;
}

/**
 * @brief Processa expressões de atribuição (operador '=').
 * Tem a menor precedência, por isso é chamada primeiro.
 */
TipoExpressao Expr_atrib() {
    printf("%s<Expr_atrib>\n", TABS);
    aumenta_ident();
    
    TipoExpressao esq = Expr_ou(); // Analisa o lado esquerdo
    
    if (t.cat == SN && t.codigo == SN_ATRIBUICAO) {
        // Verificação semântica: o lado esquerdo de '=' deve ser uma variável (L-value).
        if (!esq.eh_lvalue) {
            erro_semantico("Expressao a esquerda da atribuicao nao e um L-value valido.");
        }
        
        print_folha(t); consome(SN, SN_ATRIBUICAO);
        TipoExpressao dir = Expr_atrib(); // Analisa o lado direito
        
        gerar_load_se_necessario(dir); // Garante que o valor do lado direito esteja na pilha
        
        // Gera o código para armazenar o valor da pilha no endereço da variável
        gerar_codigo("STORE %s", esq.lexema);
        
        // Verificação semântica de tipos
        checa_compatibilidade_atribuicao(esq, dir);
        esq.tipo = dir.tipo;
        esq.eh_lvalue = 0; // O resultado de uma atribuição não é um L-value
    }
    
    diminui_ident();
    printf("%s</Expr_atrib>\n", TABS);
    return esq;
}

/**
 * @brief Processa expressões lógicas com 'OU' (||).
 */
TipoExpressao Expr_ou() {
    printf("%s<Expr_ou>\n", TABS);
    aumenta_ident();
    TipoExpressao esq = Expr_e();
    while (t.cat == SN && t.codigo == SN_OR) {
        gerar_load_se_necessario(esq);
        print_folha(t); consome(SN, SN_OR);
        TipoExpressao dir = Expr_e();
        gerar_load_se_necessario(dir);
        esq = checa_compatibilidade_operador_logico(esq, dir);
        gerar_codigo("OR");
        esq.eh_lvalue = 0;
    }
    diminui_ident();
    printf("%s</Expr_ou>\n", TABS);
    return esq;
}

/**
 * @brief Processa expressões lógicas com 'E' (&&).
 */
TipoExpressao Expr_e() {
    printf("%s<Expr_e>\n", TABS);
    aumenta_ident();
    TipoExpressao esq = Expr_relacional();
    while (t.cat == SN && t.codigo == SN_AND) {
        gerar_load_se_necessario(esq);
        print_folha(t); consome(SN, SN_AND);
        TipoExpressao dir = Expr_relacional();
        gerar_load_se_necessario(dir);
        esq = checa_compatibilidade_operador_logico(esq, dir);
        gerar_codigo("AND");
        esq.eh_lvalue = 0;
    }
    diminui_ident();
    printf("%s</Expr_e>\n", TABS);
    return esq;
}

/**
 * @brief Processa operadores relacionais (==, !=, <, >, <=, >=).
 */
TipoExpressao Expr_relacional() {
    printf("%s<Expr_relacional>\n", TABS);
    aumenta_ident();
    TipoExpressao esq = Expr_aditiva();
    
    if (t.cat == SN && (t.codigo == SN_COMPARACAO || t.codigo == SN_DIFERENTE || t.codigo == SN_MENOR || t.codigo == SN_MAIOR || t.codigo == SN_MENOR_IGUAL || t.codigo == SN_MAIOR_IGUAL)) {
        gerar_load_se_necessario(esq);
        int op = t.codigo;
        print_folha(t); consome(SN, t.codigo);
        TipoExpressao dir = Expr_aditiva();
        gerar_load_se_necessario(dir);
        esq = checa_compatibilidade_operador_relacional(esq, dir);
        // Gera o código de máquina correspondente ao operador relacional
        switch (op) {
            case SN_COMPARACAO:   gerar_codigo("EQ"); break; // Equal
            case SN_DIFERENTE:    gerar_codigo("NE"); break; // Not Equal
            case SN_MENOR:        gerar_codigo("LT"); break; // Less Than
            case SN_MAIOR:        gerar_codigo("GT"); break; // Greater Than
            case SN_MENOR_IGUAL:  gerar_codigo("LE"); break; // Less or Equal
            case SN_MAIOR_IGUAL:  gerar_codigo("GE"); break; // Greater or Equal
        }
        esq.eh_lvalue = 0;
    }
    diminui_ident();
    printf("%s</Expr_relacional>\n", TABS);
    return esq;
}

/**
 * @brief Processa operadores aditivos (+, -).
 */
TipoExpressao Expr_aditiva() {
    printf("%s<Expr_aditiva>\n", TABS);
    aumenta_ident();
    TipoExpressao esq = Expr_multiplicativa();
    while (t.cat == SN && (t.codigo == SN_SOMA || t.codigo == SN_SUBTRACAO)) {
        gerar_load_se_necessario(esq);
        int op = t.codigo;
        print_folha(t); consome(SN, t.codigo);
        TipoExpressao dir = Expr_multiplicativa();
        gerar_load_se_necessario(dir);
        esq = checa_compatibilidade_operador_aritmetico(esq, dir);
        if (op == SN_SOMA) gerar_codigo("ADD");
        else gerar_codigo("SUB");
        esq.eh_lvalue = 0;
    }
    diminui_ident();
    printf("%s</Expr_aditiva>\n", TABS);
    return esq;
}

/**
 * @brief Processa operadores multiplicativos (*, /).
 */
TipoExpressao Expr_multiplicativa() {
    printf("%s<Expr_multiplicativa>\n", TABS);
    aumenta_ident();
    TipoExpressao esq = Fator();
    while (t.cat == SN && (t.codigo == SN_MULTIPLICACAO || t.codigo == SN_DIVISAO)) {
        gerar_load_se_necessario(esq);
        int op = t.codigo;
        print_folha(t); consome(SN, t.codigo);
        TipoExpressao dir = Fator();
        gerar_load_se_necessario(dir);
        esq = checa_compatibilidade_operador_aritmetico(esq, dir);
        if (op == SN_MULTIPLICACAO) gerar_codigo("MUL");
        else gerar_codigo("DIV");
        esq.eh_lvalue = 0;
    }
    diminui_ident();
    printf("%s</Expr_multiplicativa>\n", TABS);
    return esq;
}

/**
 * @brief Processa os elementos de maior precedência em uma expressão:
 * constantes, identificadores (variáveis ou chamadas de função) e expressões entre parênteses.
 * Regra: <Fator> -> '(' <Expr> ')' | ID | CT_INT | CT_REAL | ...
 */
TipoExpressao Fator() {
    printf("%s<Fator>\n", TABS);
    aumenta_ident();
    
    TipoExpressao tipo_fator = { .tipo = NA_TIPO, .eh_lvalue = 0 };

    if (t.cat == SN && t.codigo == ABRE_PARENTESES) {
        // Expressão entre parênteses: ( <Expr> )
        print_folha(t); consome(SN, ABRE_PARENTESES);
        tipo_fator = Expr();
        gerar_load_se_necessario(tipo_fator);
        print_folha(t); consome(SN, FECHA_PARENTESES);
        tipo_fator.eh_lvalue = 0;
    } else if (t.cat == CT_INT) {
        // Constante inteira
        gerar_codigo("PUSH %d", t.valInt);
        tipo_fator.tipo = INT_;
        print_folha(t); consome(t.cat, 0);
    } else if (t.cat == CT_REAL) {
        // Constante real
        tipo_fator.tipo = REAL_;
        print_folha(t); consome(t.cat, 0);
    } else if (t.cat == ID) {
        // Identificador: pode ser uma variável ou uma chamada de função
        TokenInfo id_info = buscaDecl(t.lexema);
        if (id_info.idcategoria == PROC) {
            // Chamada de função
            char lexema_id[31];
            strcpy(lexema_id, t.lexema);
            print_folha(t); consome(ID, 0);
            
            print_folha(t); consome(SN, ABRE_PARENTESES);
            if(t.cat != SN || t.codigo != FECHA_PARENTESES) {
                Expr(); // Processa o primeiro argumento
                while(t.cat == SN && t.codigo == VIRGULA) {
                     print_folha(t); consome(SN, VIRGULA);
                     Expr(); // Processa argumentos subsequentes
                }
            }
            print_folha(t); consome(SN, FECHA_PARENTESES);
            
            gerar_codigo("CALL %s", lexema_id);
            tipo_fator.tipo = id_info.tipo;
        } else {
            // É uma variável. Apenas anota as informações. O 'LOAD' será gerado depois, se necessário.
            tipo_fator.tipo = id_info.tipo;
            strcpy(tipo_fator.lexema, id_info.lexema);
            tipo_fator.eh_lvalue = 1; // Marca como um L-value
            print_folha(t); consome(ID, 0);
        }
    } else {
        erro_semantico("Fator inesperado.");
    }
    
    diminui_ident();
    printf("%s</Fator>\n", TABS);
    return tipo_fator;
}