#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anasint.h"
#include "analex.h"
#include "tabela_simbolos.h"
#include "ansem.h"
#include "gerador_codigo.h"

// --- Variáveis Globais ---
TokenInfo tokenInfo;
char nome_funcao_atual[31];
TIPO tipo_retorno_funcao_atual;

int offset_global = 0;
int offset_local = 0;
int offset_param = 0;

// --- Protótipos de Funções Internas ---
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
void aumenta_ident() {
    if (strlen(TABS) < sizeof(TABS) - 3) strcat(TABS, "  ");
}

void diminui_ident() {
    if (strlen(TABS) >= 2) TABS[strlen(TABS) - 2] = '\0';
}

void print_folha(TOKEN tk) {
    printf("%s%s (%s)\n", TABS, tk.lexema, tk.cat == ID ? "ID" : "Palavra Reservada");
}

void consome(int categoria_esperada, int codigo_esperado) {
    if (t.cat == categoria_esperada && (codigo_esperado == 0 || t.codigo == codigo_esperado)) {
        t = Analex(fd);
    } else {
        printf("Erro na linha %d: Token inesperado.\n", contLinha);
        printf("Esperado: Categoria %d, Codigo %d\n", categoria_esperada, codigo_esperado);
        printf("Encontrado: Categoria %d, Codigo %d, Lexema '%s'\n", t.cat, t.codigo, t.lexema);
        exit(1);
    }
}

int Tipo() {
    if (t.cat == PALAVRA_RESERVADA) {
        if (t.codigo == PR_INTCON|| t.codigo == PR_REALCON || t.codigo == PR_BOOL || t.codigo == PR_CHARCON) {
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

void gerar_load_se_necessario(TipoExpressao expr) {
    if (expr.eh_lvalue) {
        TokenInfo id_info = buscaDecl(expr.lexema);
        gerar_codigo("LOAD %d, %d", id_info.escopo == GLOBAL ? 0 : 1, id_info.offset);
    }
}

void Prog() {
    printf("%s<Prog>\n", TABS);
    aumenta_ident();
    t = Analex(fd);

    gerar_codigo("INIP");
    
    while (Tipo() || (t.cat == PALAVRA_RESERVADA && t.codigo == PR_VOID)) {
        Decl_ou_Func();
    }
    
    if (buscaLexPos("main") == -1) {
        erro_semantico("Funcao 'main' nao declarada no programa.");
    }

    gerar_codigo("CALL main");
    gerar_codigo("HALT");
    
    limparTabela();
    diminui_ident();
    printf("%s</Prog>\n", TABS);
}

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
    consome(t.cat, t.codigo);
    
    strcpy(tokenInfo.lexema, t.lexema);
    strcpy(nome_funcao_atual, t.lexema);
    tokenInfo.tipo = tipo_atual;

    print_folha(t);
    consome(ID, 0);

    if (t.cat == SN && t.codigo == ABRE_PARENTESES) {
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
        
        matarZumbis(func_pos);
        retirarLocais();
        
        gerar_codigo("RET 0, %d", num_params);
    } 
    else {
        tokenInfo.idcategoria = VAR_GLOBAL;
        tokenInfo.escopo = GLOBAL;
        tokenInfo.offset = offset_global++;
        inserirNaTabela(tokenInfo);
        Decl_var_body();
        gerar_codigo("AMEM 1");
    }
    diminui_ident();
    printf("%s</Decl_ou_Func>\n", TABS);
}

int Func_body(int procPos) {
    printf("%s<Func_body>\n", TABS);
    aumenta_ident();
    
    print_folha(t);
    consome(SN, ABRE_PARENTESES);
    
    offset_local = 0;
    offset_param = 0;

    int num_params = 0;
    if (t.cat != SN || t.codigo != FECHA_PARENTESES) {
        num_params = Tipos_param(procPos);
    }

    print_folha(t);
    consome(SN, FECHA_PARENTESES);

    if (strcmp(nome_funcao_atual, "main") != 0) {
        gerar_codigo("INIPR 1");
    }

    if (t.cat == SN && t.codigo == PONTO_VIRGULA) {
        print_folha(t);
        consome(SN, PONTO_VIRGULA);
    } else {
        print_folha(t);
        consome(SN, ABRE_CHAVES);
        
        int contador_locais = 0;
        while (Tipo()) {
            tokenInfo.idcategoria = VAR_LOCAL;
            tokenInfo.escopo = LOCAL;
            Decl();
            contador_locais++;
        }
        
        if (contador_locais > 0) {
            gerar_codigo("AMEM %d", contador_locais);
        }
        
        while (!(t.cat == SN && t.codigo == FECHA_CHAVES)) {
            Cmd();
        }
        
        if (contador_locais > 0) {
            gerar_codigo("DMEM %d", contador_locais);
        }
        
        print_folha(t);
        consome(SN, FECHA_CHAVES);
    }
    diminui_ident();
    printf("%s</Func_body>\n", TABS);
    return num_params;
}

void Decl_var_body() {
    if (t.cat == SN && t.codigo == PONTO_VIRGULA) {
        print_folha(t);
        consome(SN, PONTO_VIRGULA);
    }
}

void Decl() {
    printf("%s<Decl>\n", TABS);
    aumenta_ident();
    
    if (Tipo()) {
        print_folha(t);
        consome(PALAVRA_RESERVADA, t.codigo);
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

void Decl_var() {
    printf("%s<Decl_var>\n", TABS);
    aumenta_ident();
    
    strcpy(tokenInfo.lexema, t.lexema);
    print_folha(t);
    consome(ID, 0);

    if (tokenInfo.idcategoria == VAR_LOCAL) {
        tokenInfo.offset = offset_local++;
    } else if (tokenInfo.idcategoria == PROC_PAR) {
        tokenInfo.offset = offset_param++;
    }

    if (t.cat == SN && t.codigo == ABRE_COLCHETES) {
        print_folha(t); consome(SN, ABRE_COLCHETES);
        print_folha(t); consome(CT_INT, 0);
        print_folha(t); consome(SN, FECHA_COLCHETES);
    }
    
    inserirNaTabela(tokenInfo);
    
    diminui_ident();
    printf("%s</Decl_var>\n", TABS);
}

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


void Cmd() {
    printf("%s<Cmd>\n", TABS);
    aumenta_ident();
    
    if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_IF) {
        print_folha(t); consome(PALAVRA_RESERVADA, PR_IF);
        print_folha(t); consome(SN, ABRE_PARENTESES);
        TipoExpressao expr = Expr();
        gerar_load_se_necessario(expr);
        print_folha(t); consome(SN, FECHA_PARENTESES);
        
        char rotulo_falso[10];
        novo_rotulo(rotulo_falso);
        gerar_codigo("GOFALSE %s", rotulo_falso);

        Cmd();

        if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_ELSE) {
            char rotulo_fim[10];
            novo_rotulo(rotulo_fim);
            gerar_codigo("GOTO %s", rotulo_fim);
            gerar_codigo("LABEL %s", rotulo_falso);

            print_folha(t); consome(PALAVRA_RESERVADA, PR_ELSE);
            Cmd();
            gerar_codigo("LABEL %s", rotulo_fim);
        } else {
            gerar_codigo("LABEL %s", rotulo_falso);
        }
    } else if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_WHILE) {
        char rotulo_inicio[10], rotulo_fim[10];
        novo_rotulo(rotulo_inicio);
        novo_rotulo(rotulo_fim);

        gerar_codigo("LABEL %s", rotulo_inicio);
        
        print_folha(t); consome(PALAVRA_RESERVADA, PR_WHILE);
        print_folha(t); consome(SN, ABRE_PARENTESES);
        TipoExpressao expr = Expr();
        gerar_load_se_necessario(expr);
        gerar_codigo("GOFALSE %s", rotulo_fim);
        print_folha(t); consome(SN, FECHA_PARENTESES);
        Cmd();
        gerar_codigo("GOTO %s", rotulo_inicio);
        gerar_codigo("LABEL %s", rotulo_fim);

    } else if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_RETURN) {
        print_folha(t); consome(PALAVRA_RESERVADA, PR_RETURN);
        if (t.cat != SN || t.codigo != PONTO_VIRGULA) {
            TipoExpressao expr = Expr();
            gerar_load_se_necessario(expr);
        } else {
             gerar_codigo("PUSH 0"); // Default return for main
        }
        print_folha(t); consome(SN, PONTO_VIRGULA);
    } else if (t.cat == SN && t.codigo == ABRE_CHAVES) {
        print_folha(t); consome(SN, ABRE_CHAVES);
        while (t.cat != SN || t.codigo != FECHA_CHAVES) {
            Cmd();
        }
        print_folha(t); consome(SN, FECHA_CHAVES);
    } else {
        Expr();
        print_folha(t); consome(SN, PONTO_VIRGULA);
    }
    
    diminui_ident();
    printf("%s</Cmd>\n", TABS);
}

TipoExpressao Expr() {
    printf("%s<Expr>\n", TABS);
    aumenta_ident();
    TipoExpressao tipo = Expr_atrib();
    diminui_ident();
    printf("%s</Expr>\n", TABS);
    return tipo;
}

TipoExpressao Expr_atrib() {
    printf("%s<Expr_atrib>\n", TABS);
    aumenta_ident();
    
    TipoExpressao esq = Expr_ou();
    
    if (t.cat == SN && t.codigo == SN_ATRIBUICAO) {
        if (!esq.eh_lvalue) {
            erro_semantico("Expressao a esquerda da atribuicao nao e um L-value valido.");
        }
        
        print_folha(t); consome(SN, SN_ATRIBUICAO);
        TipoExpressao dir = Expr_atrib();
        
        gerar_load_se_necessario(dir);
        gerar_codigo("STORE %s", esq.lexema);
        
        checa_compatibilidade_atribuicao(esq, dir);
        esq.tipo = dir.tipo;
        esq.eh_lvalue = 0;
    }
    
    diminui_ident();
    printf("%s</Expr_atrib>\n", TABS);
    return esq;
}

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
        switch (op) {
            case SN_COMPARACAO: gerar_codigo("EQ"); break;
            case SN_DIFERENTE: gerar_codigo("NE"); break;
            case SN_MENOR: gerar_codigo("LT"); break;
            case SN_MAIOR: gerar_codigo("GT"); break;
            case SN_MENOR_IGUAL: gerar_codigo("LE"); break;
            case SN_MAIOR_IGUAL: gerar_codigo("GE"); break;
        }
        esq.eh_lvalue = 0;
    }
    diminui_ident();
    printf("%s</Expr_relacional>\n", TABS);
    return esq;
}

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

TipoExpressao Fator() {
    printf("%s<Fator>\n", TABS);
    aumenta_ident();
    
    TipoExpressao tipo_fator = { .tipo = NA_TIPO, .eh_lvalue = 0 };

    if (t.cat == SN && t.codigo == ABRE_PARENTESES) {
        print_folha(t); consome(SN, ABRE_PARENTESES);
        tipo_fator = Expr();
        gerar_load_se_necessario(tipo_fator);
        print_folha(t); consome(SN, FECHA_PARENTESES);
        tipo_fator.eh_lvalue = 0;
    } else if (t.cat == CT_INT) {
        gerar_codigo("PUSH %d", t.valInt);
        tipo_fator.tipo = INT_;
        print_folha(t); consome(t.cat, 0);
    } else if (t.cat == CT_REAL) {
        tipo_fator.tipo = REAL_;
        print_folha(t); consome(t.cat, 0);
    } else if (t.cat == ID) {
        TokenInfo id_info = buscaDecl(t.lexema);
        if (id_info.idcategoria == PROC) {
            char lexema_id[31];
            strcpy(lexema_id, t.lexema);
            print_folha(t); consome(ID, 0);
            
            print_folha(t); consome(SN, ABRE_PARENTESES);
            if(t.cat != SN || t.codigo != FECHA_PARENTESES) {
                Expr();
                while(t.cat == SN && t.codigo == VIRGULA) {
                     print_folha(t); consome(SN, VIRGULA);
                     Expr();
                }
            }
            print_folha(t); consome(SN, FECHA_PARENTESES);
            
            gerar_codigo("CALL %s", lexema_id);
            tipo_fator.tipo = id_info.tipo;
        } else {
            tipo_fator.tipo = id_info.tipo;
            strcpy(tipo_fator.lexema, id_info.lexema);
            tipo_fator.eh_lvalue = 1;
            print_folha(t); consome(ID, 0);
        }
    } else {
        erro_semantico("Fator inesperado.");
    }
    
    diminui_ident();
    printf("%s</Fator>\n", TABS);
    return tipo_fator;
}