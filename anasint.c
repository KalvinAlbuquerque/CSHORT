#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anasint.h"
#include "analex.h"
#include "tabela_simbolos.h"
#include "ansem.h"

// --- Variáveis Globais ---
TokenInfo tokenInfo;
char nome_funcao_atual[31];
TIPO tipo_retorno_funcao_atual;

// --- Protótipos de Funções Internas ---
void Prog();
void Decl_ou_Func();
void Func_body(int procPos);
void Decl();
void Decl_var_body();
void Decl_var();
int Tipo();
void Tipos_param(int func_pos);
void Cmd();
TipoExpressao Expr();
TipoExpressao Expr_atrib();
TipoExpressao Expr_ou();
TipoExpressao Expr_e();
TipoExpressao Expr_relacional();
TipoExpressao Expr_aditiva();
TipoExpressao Expr_multiplicativa();
TipoExpressao Fator();

// --- Funções Auxiliares de Identação e Impressão (sem alterações) ---
void aumenta_ident() { if (strlen(TABS) < sizeof(TABS) - 3) strcat(TABS, "  "); }
void diminui_ident() { if (strlen(TABS) >= 2) TABS[strlen(TABS) - 2] = '\0'; }

void print_folha(TOKEN tk) {
    printf("%s- ", TABS);
    switch (tk.cat) {
        case ID: printf("ID: %s\n", tk.lexema); break;
        case SN: printf("SN: %d\n", tk.codigo); break;
        case CT_INT: printf("CT_INT: %d\n", tk.valInt); break;
        case CT_REAL: printf("CT_REAL: %f\n", tk.valReal); break;
        case CT_CHAR: printf("CT_CHAR: '%c'\n", tk.valInt); break;
        case CT_STRING: printf("CT_STRING: \"%s\"\n", tk.lexema); break;
        case PALAVRA_RESERVADA: printf("PR: %d\n", tk.codigo); break;
        default: printf("TOKEN (cat %d)\n", tk.cat); break;
    }
}

// --- Função `consome` (sem alterações) ---
void consome(int categoria_esperada, int codigo_esperado) {
    if (t.cat == categoria_esperada && (codigo_esperado == 0 || t.codigo == codigo_esperado)) {
        t = Analex(fd);
    } else {
        char msg_erro[256];
        sprintf(msg_erro, "Token inesperado. Esperado %d, mas encontrado %d.", categoria_esperada, t.cat);
        error(msg_erro);
    }
}

// --- Funções de Análise Sintática com Semântica Integrada ---

int Tipo() {
    if (t.cat == PALAVRA_RESERVADA) {
        switch (t.codigo) {
            case PR_INTCON: tokenInfo.tipo = INT_; return 1;
            case PR_CHARCON: tokenInfo.tipo = CHAR_; return 1;
            case PR_FLOAT: tokenInfo.tipo = REAL_; return 1;
            case PR_BOOL: tokenInfo.tipo = BOOL_; return 1;
            default: return 0;
        }
    }
    return 0;
}

void Prog() {
    printf("%s<Prog>\n", TABS); aumenta_ident();
    t = Analex(fd);
    while (t.cat != FIM_ARQ) {
        if (Tipo() || (t.cat == PALAVRA_RESERVADA && t.codigo == PR_VOID)) {
            tokenInfo.escopo = GLOBAL;
            Decl_ou_Func();
        } else {
            error("Esperado uma declaracao de variavel ou definicao de funcao no escopo global.");
        }
    }
    // Checagem semântica final: verificar se a função main foi declarada
    if (buscaLexPos("main") == -1) {
        erro_semantico("Funcao 'main' nao declarada no programa.");
    }
    limparTabela();
    diminui_ident(); printf("%s</Prog>\n", TABS);
}

void Decl_ou_Func() {
    printf("%s<Decl_ou_Func>\n", TABS); aumenta_ident();
    int tipo_atual = tokenInfo.tipo;
    if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_VOID) {
        tipo_atual = NA_TIPO;
    }

    print_folha(t); consome(t.cat, t.codigo);

    // Salva nome e tipo para uso futuro (em funções)
    strcpy(tokenInfo.lexema, t.lexema);
    strcpy(nome_funcao_atual, t.lexema);
    tokenInfo.tipo = tipo_atual;

    print_folha(t); consome(ID, 0);

    if (t.cat == SN && t.codigo == ABRE_PARENTESES) {
        tokenInfo.idcategoria = PROC;
        tipo_retorno_funcao_atual = tokenInfo.tipo;
        int func_pos = tabela.topo;
        inserirNaTabela(tokenInfo);
        Func_body(func_pos);
    } else {
        tokenInfo.idcategoria = VAR_GLOBAL;
        inserirNaTabela(tokenInfo);
        Decl_var_body();
    }
    diminui_ident(); printf("%s</Decl_ou_Func>\n", TABS);
}

void Func_body(int procPos) {
    printf("%s<Func_body>\n", TABS); aumenta_ident();
    print_folha(t); consome(SN, ABRE_PARENTESES);
    tokenInfo.escopo = LOCAL;

    if (t.cat != SN || t.codigo != FECHA_PARENTESES) {
        Tipos_param(procPos);
    }

    print_folha(t); consome(SN, FECHA_PARENTESES);

    if (t.cat == SN && t.codigo == PONTO_VIRGULA) { // Protótipo de função
        print_folha(t); consome(SN, PONTO_VIRGULA);
    } else { // Corpo da função
        print_folha(t); consome(SN, ABRE_CHAVES);
        while (Tipo()) {
            tokenInfo.idcategoria = VAR_LOCAL;
            Decl();
        }
        while (!(t.cat == SN && t.codigo == FECHA_CHAVES)) {
            Cmd();
        }
        print_folha(t); consome(SN, FECHA_CHAVES);
        matarZumbis(procPos);
        retirarLocais();
    }
    diminui_ident(); printf("%s</Func_body>\n", TABS);
}

void Decl_var_body() {
    printf("%s<Decl_var_body>\n", TABS); aumenta_ident();
    if (t.cat == SN && t.codigo == ABRE_COLCHETES) {
        print_folha(t); consome(SN, ABRE_COLCHETES);
        print_folha(t); consome(CT_INT, 0);
        print_folha(t); consome(SN, FECHA_COLCHETES);
    }

    while (t.cat == SN && t.codigo == VIRGULA) {
        print_folha(t); consome(SN, VIRGULA);
        Decl_var();
    }
    print_folha(t); consome(SN, PONTO_VIRGULA);
    diminui_ident(); printf("%s</Decl_var_body>\n", TABS);
}

void Decl() {
    printf("%s<Decl>\n", TABS); aumenta_ident();
    if (Tipo()) {
        int tipo_linha = tokenInfo.tipo;
        print_folha(t); consome(t.cat, t.codigo);
        tokenInfo.tipo = tipo_linha;
        Decl_var();

        while (t.cat == SN && t.codigo == VIRGULA) {
            print_folha(t); consome(SN, VIRGULA);
            tokenInfo.tipo = tipo_linha;
            tokenInfo.idcategoria = VAR_LOCAL;
            Decl_var();
        }
        print_folha(t); consome(SN, PONTO_VIRGULA);
    } else {
        error("Esperado uma declaracao de variavel local.");
    }
    diminui_ident(); printf("%s</Decl>\n", TABS);
}

void Decl_var() {
    printf("%s<Decl_var>\n", TABS); aumenta_ident();
    strcpy(tokenInfo.lexema, t.lexema);
    print_folha(t); consome(ID, 0);

    if (t.cat == SN && t.codigo == ABRE_COLCHETES) {
        print_folha(t); consome(SN, ABRE_COLCHETES);
        print_folha(t); consome(CT_INT, 0);
        print_folha(t); consome(SN, FECHA_COLCHETES);
    }
    inserirNaTabela(tokenInfo);
    diminui_ident(); printf("%s</Decl_var>\n", TABS);
}

void Tipos_param(int func_pos) {
    printf("%s<Tipos_param>\n", TABS); aumenta_ident();
    if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_VOID) {
        print_folha(t); consome(PALAVRA_RESERVADA, PR_VOID);
    } else {
        while (Tipo()) {
            int tipo_param = tokenInfo.tipo;
            print_folha(t); consome(t.cat, t.codigo);

            tokenInfo.tipo = tipo_param;
            tokenInfo.idcategoria = PROC_PAR;
            tokenInfo.escopo = LOCAL;
            strcpy(tokenInfo.lexema, t.lexema);
            print_folha(t); consome(ID, 0);

            if (t.cat == SN && t.codigo == ABRE_COLCHETES) {
                print_folha(t); consome(SN, ABRE_COLCHETES);
                print_folha(t); consome(SN, FECHA_COLCHETES);
            }
            inserirNaTabela(tokenInfo);

            if (t.cat == SN && t.codigo == VIRGULA) {
                print_folha(t); consome(SN, VIRGULA);
            } else {
                break;
            }
        }
    }
    diminui_ident(); printf("%s</Tipos_param>\n", TABS);
}

void Cmd() {
    printf("%s<Cmd>\n", TABS); aumenta_ident();
    if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_IF) {
        print_folha(t); consome(PALAVRA_RESERVADA, PR_IF);
        print_folha(t); consome(SN, ABRE_PARENTESES);
        TipoExpressao expr = Expr();
        checa_condicao_booleana("if", expr);
        print_folha(t); consome(SN, FECHA_PARENTESES);
        Cmd();
        if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_ELSE) {
            print_folha(t); consome(PALAVRA_RESERVADA, PR_ELSE);
            Cmd();
        }
    } else if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_WHILE) {
        print_folha(t); consome(PALAVRA_RESERVADA, PR_WHILE);
        print_folha(t); consome(SN, ABRE_PARENTESES);
        TipoExpressao expr = Expr();
        checa_condicao_booleana("while", expr);
        print_folha(t); consome(SN, FECHA_PARENTESES);
        Cmd();
    } else if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_FOR) {
        print_folha(t); consome(PALAVRA_RESERVADA, PR_FOR);
        print_folha(t); consome(SN, ABRE_PARENTESES);
        if (t.cat != SN || t.codigo != PONTO_VIRGULA) { Expr_atrib(); }
        print_folha(t); consome(SN, PONTO_VIRGULA);
        if (t.cat != SN || t.codigo != PONTO_VIRGULA) {
            TipoExpressao expr = Expr();
            checa_condicao_booleana("for", expr);
        }
        print_folha(t); consome(SN, PONTO_VIRGULA);
        if (t.cat != SN || t.codigo != FECHA_PARENTESES) { Expr_atrib(); }
        print_folha(t); consome(SN, FECHA_PARENTESES);
        Cmd();
    } else if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_RETURN) {
        print_folha(t); consome(PALAVRA_RESERVADA, PR_RETURN);
        TipoExpressao expr_ret;
        expr_ret.tipo = NA_TIPO;
        int tem_expr = 0;
        if (t.cat != SN || t.codigo != PONTO_VIRGULA) {
            expr_ret = Expr();
            tem_expr = 1;
        }
        checa_retorno_funcao(tipo_retorno_funcao_atual, nome_funcao_atual, expr_ret, tem_expr);
        print_folha(t); consome(SN, PONTO_VIRGULA);
    } else if (t.cat == SN && t.codigo == ABRE_CHAVES) {
        print_folha(t); consome(SN, ABRE_CHAVES);
        while (!(t.cat == SN && t.codigo == FECHA_CHAVES)) {
            Cmd();
        }
        print_folha(t); consome(SN, FECHA_CHAVES);
    } else if (t.cat == SN && t.codigo == PONTO_VIRGULA) {
        print_folha(t); consome(SN, PONTO_VIRGULA);
    } else {
        Expr();
        print_folha(t); consome(SN, PONTO_VIRGULA);
    }
    diminui_ident(); printf("%s</Cmd>\n", TABS);
}

TipoExpressao Expr() {
    printf("%s<Expr>\n", TABS); aumenta_ident();
    TipoExpressao tipo = Expr_atrib();
    diminui_ident(); printf("%s</Expr>\n", TABS);
    return tipo;
}

TipoExpressao Expr_atrib() {
    printf("%s<Expr_atrib>\n", TABS); aumenta_ident();
    TipoExpressao esq = Expr_ou();
    if (t.cat == SN && t.codigo == SN_ATRIBUICAO) {
        print_folha(t); consome(SN, SN_ATRIBUICAO);
        TipoExpressao dir = Expr_atrib();
        checa_compatibilidade_atribuicao(esq, dir);
        esq.tipo = dir.tipo;
        esq.eh_lvalue = 0;
        strcpy(esq.lexema, "");
    }
    diminui_ident(); printf("%s</Expr_atrib>\n", TABS);
    return esq;
}

TipoExpressao Expr_ou() {
    printf("%s<Expr_ou>\n", TABS); aumenta_ident();
    TipoExpressao esq = Expr_e();
    while (t.cat == SN && t.codigo == SN_OR) {
        print_folha(t); consome(SN, SN_OR);
        TipoExpressao dir = Expr_e();
        esq = checa_compatibilidade_operador_logico(esq, dir);
    }
    diminui_ident(); printf("%s</Expr_ou>\n", TABS);
    return esq;
}

TipoExpressao Expr_e() {
    printf("%s<Expr_e>\n", TABS); aumenta_ident();
    TipoExpressao esq = Expr_relacional();
    while (t.cat == SN && t.codigo == SN_AND) {
        print_folha(t); consome(SN, SN_AND);
        TipoExpressao dir = Expr_relacional();
        esq = checa_compatibilidade_operador_logico(esq, dir);
    }
    diminui_ident(); printf("%s</Expr_e>\n", TABS);
    return esq;
}

TipoExpressao Expr_relacional() {
    printf("%s<Expr_relacional>\n", TABS); aumenta_ident();
    TipoExpressao esq = Expr_aditiva();
    if (t.cat == SN && (t.codigo == SN_COMPARACAO || t.codigo == SN_DIFERENTE || t.codigo == SN_MAIOR || t.codigo == SN_MENOR || t.codigo == SN_MAIOR_IGUAL || t.codigo == SN_MENOR_IGUAL)) {
        print_folha(t); consome(SN, t.codigo);
        TipoExpressao dir = Expr_aditiva();
        esq = checa_compatibilidade_operador_relacional(esq, dir);
    }
    diminui_ident(); printf("%s</Expr_relacional>\n", TABS);
    return esq;
}

TipoExpressao Expr_aditiva() {
    printf("%s<Expr_aditiva>\n", TABS); aumenta_ident();
    TipoExpressao esq = Expr_multiplicativa();
    while (t.cat == SN && (t.codigo == SN_SOMA || t.codigo == SN_SUBTRACAO)) {
        print_folha(t); consome(SN, t.codigo);
        TipoExpressao dir = Expr_multiplicativa();
        esq = checa_compatibilidade_operador_aritmetico(esq, dir);
    }
    diminui_ident(); printf("%s</Expr_aditiva>\n", TABS);
    return esq;
}

TipoExpressao Expr_multiplicativa() {
    printf("%s<Expr_multiplicativa>\n", TABS); aumenta_ident();
    TipoExpressao esq = Fator();
    while (t.cat == SN && (t.codigo == SN_MULTIPLICACAO || t.codigo == SN_DIVISAO)) {
        print_folha(t); consome(SN, t.codigo);
        TipoExpressao dir = Fator();
        esq = checa_compatibilidade_operador_aritmetico(esq, dir);
    }
    diminui_ident(); printf("%s</Expr_multiplicativa>\n", TABS);
    return esq;
}

TipoExpressao Fator() {
    printf("%s<Fator>\n", TABS); aumenta_ident();
    TipoExpressao tipo_fator;
    tipo_fator.eh_lvalue = 0;
    tipo_fator.eh_constante = 0;
    strcpy(tipo_fator.lexema, "");

    if (t.cat == SN && (t.codigo == SN_SOMA || t.codigo == SN_SUBTRACAO)) {
        print_folha(t); consome(SN, t.codigo);
        tipo_fator = Fator();
        tipo_fator.eh_lvalue = 0;
    } else if (t.cat == SN && t.codigo == SN_NEGACAO) {
        print_folha(t); consome(SN, t.codigo);
        tipo_fator = Fator();
        if(tipo_fator.tipo != BOOL_ && tipo_fator.tipo != INT_){
            erro_semantico("Operador de negacao '!' exige tipo compativel com booleano, mas foi recebido '%s'.", T_tipo[tipo_fator.tipo]);
        }
        tipo_fator.tipo = BOOL_;
        tipo_fator.eh_lvalue = 0;
    } else if (t.cat == ID) {
        TokenInfo id_info = buscaDecl(t.lexema);
        if (id_info.zumbi == ZUMBI_) {
            erro_semantico("A variavel '%s' esta fora de escopo.", t.lexema);
        }
        tipo_fator.tipo = id_info.tipo;
        tipo_fator.eh_lvalue = (id_info.idcategoria != PROC);
        strcpy(tipo_fator.lexema, t.lexema);

        char lexema_id[31];
        strcpy(lexema_id, t.lexema);
        print_folha(t); consome(ID, 0);

        if (t.cat == SN && t.codigo == ABRE_COLCHETES) {
            print_folha(t); consome(SN, ABRE_COLCHETES);
            TipoExpressao indice = Expr();
            if (indice.tipo != INT_ && indice.tipo != CHAR_) {
                erro_semantico("O indice do array '%s' deve ser um inteiro, mas foi recebido '%s'.", lexema_id, T_tipo[indice.tipo]);
            }
            print_folha(t); consome(SN, FECHA_COLCHETES);
        } else if (t.cat == SN && t.codigo == ABRE_PARENTESES) {
             if (id_info.idcategoria != PROC) {
                erro_semantico("O identificador '%s' nao e uma funcao e nao pode ser chamado.", lexema_id);
            }
            tipo_fator.eh_lvalue = 0;
            strcpy(tipo_fator.lexema, "");
            print_folha(t); consome(SN, ABRE_PARENTESES);
            if (!(t.cat == SN && t.codigo == FECHA_PARENTESES)) {
                Expr();
                while (t.cat == SN && t.codigo == VIRGULA) {
                    print_folha(t); consome(SN, VIRGULA);
                    Expr();
                }
            }
            print_folha(t); consome(SN, FECHA_PARENTESES);
        }
    } else if (t.cat == CT_INT || t.cat == CT_CHAR || t.cat == CT_REAL) {
        tipo_fator.eh_constante = 1;
        if (t.cat == CT_INT) tipo_fator.tipo = INT_;
        else if (t.cat == CT_CHAR) tipo_fator.tipo = CHAR_;
        else tipo_fator.tipo = REAL_;
        print_folha(t); consome(t.cat, 0);
    } else if (t.cat == SN && t.codigo == ABRE_PARENTESES) {
        print_folha(t); consome(SN, ABRE_PARENTESES);
        tipo_fator = Expr();
        print_folha(t); consome(SN, FECHA_PARENTESES);
    } else {
        error("Fator mal formado. Esperado ID, constante ou '('");
    }
    diminui_ident(); printf("%s</Fator>\n", TABS);
    return tipo_fator;
}