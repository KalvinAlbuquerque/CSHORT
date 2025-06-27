// Anasint.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Necessário para strcat e strlen
#include "anasint.h"
#include "analex.h"

// --- Protótipos de todas as funções para evitar warnings do compilador ---
void Prog();
void Decl();
void Decl_var();
int  Tipo();
void Tipos_param();
void Cmd();
void Decl_ou_Func();
void Func_body();
void Decl_var_body();
void Expr();
void Expr_atrib();
void Expr_ou();
void Expr_e();
void Expr_relacional();
void Expr_aditiva();
void Expr_multiplicativa();
void Fator();
void print_folha(TOKEN tk);
void aumenta_ident();
void diminui_ident();

// --- Funções Auxiliares para Visualização ---

// Adiciona dois espaços à string de indentação para um visual mais limpo
void aumenta_ident() {
    // Verifica se há espaço no buffer para evitar overflow
    if (strlen(TABS) < sizeof(TABS) - 3) {
        strcat(TABS, "  ");
    }
}

// Remove os últimos dois espaços da string de indentação
void diminui_ident() {
    if (strlen(TABS) >= 2) {
        TABS[strlen(TABS) - 2] = '\0';
    }
}

// Imprime um token terminal (folha da árvore) de forma legível
void print_folha(TOKEN tk) {
    printf("%s- ", TABS); // Usa um traço para indicar que é uma folha
    switch (tk.cat) {
        case ID: printf("ID: %s\n", tk.lexema); break;
        case SN: printf("SN: %d\n", tk.codigo); break;
        case CT_INT: printf("CT_INT: %d\n", tk.valInt); break;
        case CT_REAL: printf("CT_REAL: %f\n", tk.valReal); break;
        case PALAVRA_RESERVADA: printf("PR: %d\n", tk.codigo); break;
        default: printf("TOKEN (cat %d)\n", tk.cat); break;
    }
}

// --- Função `consome` ---
void consome(int categoria_esperada, int codigo_esperado) {
    if (t.cat == categoria_esperada && (codigo_esperado == 0 || t.codigo == codigo_esperado)) {
        // A impressão agora é feita pela função que chama `consome`, para maior controle
        t = Analex(fd);
    } else {
        char msg_erro[200];
        sprintf(msg_erro, "Erro sintatico na linha %d: esperado (cat %d, cod %d), mas encontrei (cat %d, cod %d)",
                contLinha, categoria_esperada, codigo_esperado, t.cat, t.codigo);
        error(msg_erro);
    }
}

// --- Funções do Analisador Sintático com Visualização Limpa ---

int Tipo() {
    if (t.cat == PALAVRA_RESERVADA) {
        switch (t.codigo) {
            case PR_INTCON: case PR_CHARCON: case PR_REALCON:
            case PR_STRINGCON: case PR_BOOL: case PR_FLOAT:
                return 1;
            default:
                return 0;
        }
    }
    return 0;
}

void Prog() {
    printf("%s<Prog>\n", TABS); aumenta_ident();
    t = Analex(fd);
    while (t.cat != FIM_ARQ) {
        if (Tipo() || (t.cat == PALAVRA_RESERVADA && t.codigo == PR_VOID)) {
            Decl_ou_Func();
        } else {
            error("Erro sintatico: esperado uma declaracao ou uma funcao.");
        }
    }
    diminui_ident(); printf("%s</Prog>\n", TABS);
}

void Decl_ou_Func() {
    printf("%s<Decl_ou_Func>\n", TABS); aumenta_ident();
    print_folha(t); consome(t.cat, t.codigo);
    print_folha(t); consome(ID, 0);

    if (t.cat == SN && t.codigo == ABRE_PARENTESES) {
        Func_body();
    } else {
        Decl_var_body();
    }
    diminui_ident(); printf("%s</Decl_ou_Func>\n", TABS);
}

void Func_body() {
    printf("%s<Func_body>\n", TABS); aumenta_ident();
    print_folha(t); consome(SN, ABRE_PARENTESES);
    Tipos_param();
    print_folha(t); consome(SN, FECHA_PARENTESES);
    print_folha(t); consome(SN, ABRE_CHAVES);
    while (Tipo()) {
        Decl();
    }
    while (!(t.cat == SN && t.codigo == FECHA_CHAVES)) {
        Cmd();
    }
    print_folha(t); consome(SN, FECHA_CHAVES);
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
        Decl_var(); // Decl_var agora consome o tipo
        while (t.cat == SN && t.codigo == VIRGULA) {
            print_folha(t); consome(SN, VIRGULA);
            print_folha(t); consome(ID, 0); // Consome o ID da próxima variável
        }
        print_folha(t); consome(SN, PONTO_VIRGULA);
    } else {
        error("Declaracao de variavel local esperada.");
    }
    diminui_ident(); printf("%s</Decl>\n", TABS);
}

void Decl_var() {
    printf("%s<Decl_var>\n", TABS); aumenta_ident();
    print_folha(t); consome(t.cat, t.codigo); // Consome o tipo
    print_folha(t); consome(ID, 0); // Consome o ID
    
    if (t.cat == SN && t.codigo == ABRE_COLCHETES) {
        print_folha(t); consome(SN, ABRE_COLCHETES);
        print_folha(t); consome(CT_INT, 0);
        print_folha(t); consome(SN, FECHA_COLCHETES);
    }
    diminui_ident(); printf("%s</Decl_var>\n", TABS);
}

void Tipos_param() {
    printf("%s<Tipos_param>\n", TABS); aumenta_ident();
    if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_VOID) {
        print_folha(t); consome(PALAVRA_RESERVADA, PR_VOID);
    } else if (t.cat != SN || t.codigo != FECHA_PARENTESES) {
        if (Tipo()) {
            print_folha(t); consome(t.cat, t.codigo);
            print_folha(t); consome(ID, 0);
            if (t.cat == SN && t.codigo == ABRE_COLCHETES) {
                print_folha(t); consome(SN, ABRE_COLCHETES);
                print_folha(t); consome(SN, FECHA_COLCHETES);
            }
            while (t.cat == SN && t.codigo == VIRGULA) {
                print_folha(t); consome(SN, VIRGULA);
                if (Tipo()) {
                    print_folha(t); consome(t.cat, t.codigo);
                    print_folha(t); consome(ID, 0);
                    if (t.cat == SN && t.codigo == ABRE_COLCHETES) {
                        print_folha(t); consome(SN, ABRE_COLCHETES);
                        print_folha(t); consome(SN, FECHA_COLCHETES);
                    }
                } else {
                    error("Esperado um tipo no parametro da funcao.");
                }
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
        Expr();
        print_folha(t); consome(SN, FECHA_PARENTESES);
        Cmd();
        if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_ELSE) {
            print_folha(t); consome(PALAVRA_RESERVADA, PR_ELSE);
            Cmd();
        }
    } else if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_WHILE) {
        print_folha(t); consome(PALAVRA_RESERVADA, PR_WHILE);
        print_folha(t); consome(SN, ABRE_PARENTESES);
        Expr();
        print_folha(t); consome(SN, FECHA_PARENTESES);
        Cmd();
    } else if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_RETURN) {
        print_folha(t); consome(PALAVRA_RESERVADA, PR_RETURN);
        if (!(t.cat == SN && t.codigo == PONTO_VIRGULA)) {
            Expr();
        }
        print_folha(t); consome(SN, PONTO_VIRGULA);
    } else if (t.cat == SN && t.codigo == ABRE_CHAVES) {
        print_folha(t); consome(SN, ABRE_CHAVES);
        while (!(t.cat == SN && t.codigo == FECHA_CHAVES)) {
            Cmd();
        }
        print_folha(t); consome(SN, FECHA_CHAVES);
    } else {
        Expr();
        print_folha(t); consome(SN, PONTO_VIRGULA);
    }
    diminui_ident(); printf("%s</Cmd>\n", TABS);
}

void Expr() {
    printf("%s<Expr>\n", TABS); aumenta_ident();
    Expr_atrib();
    diminui_ident(); printf("%s</Expr>\n", TABS);
}

void Expr_atrib() {
    printf("%s<Expr_atrib>\n", TABS); aumenta_ident();
    Expr_ou();
    if (t.cat == SN && t.codigo == SN_ATRIBUICAO) {
        print_folha(t); consome(SN, SN_ATRIBUICAO);
        Expr_atrib();
    }
    diminui_ident(); printf("%s</Expr_atrib>\n", TABS);
}

void Expr_ou() {
    printf("%s<Expr_ou>\n", TABS); aumenta_ident();
    Expr_e();
    while (t.cat == SN && t.codigo == SN_OR) {
        print_folha(t); consome(SN, SN_OR);
        Expr_e();
    }
    diminui_ident(); printf("%s</Expr_ou>\n", TABS);
}

void Expr_e() {
    printf("%s<Expr_e>\n", TABS); aumenta_ident();
    Expr_relacional();
    while (t.cat == SN && t.codigo == SN_AND) {
        print_folha(t); consome(SN, SN_AND);
        Expr_relacional();
    }
    diminui_ident(); printf("%s</Expr_e>\n", TABS);
}

void Expr_relacional() {
    printf("%s<Expr_relacional>\n", TABS); aumenta_ident();
    Expr_aditiva();
    while (t.cat == SN && (t.codigo == SN_COMPARACAO || t.codigo == SN_DIFERENTE || t.codigo == SN_MAIOR || t.codigo == SN_MENOR || t.codigo == SN_MAIOR_IGUAL || t.codigo == SN_MENOR_IGUAL)) {
        print_folha(t); consome(SN, t.codigo);
        Expr_aditiva();
    }
    diminui_ident(); printf("%s</Expr_relacional>\n", TABS);
}

void Expr_aditiva() {
    printf("%s<Expr_aditiva>\n", TABS); aumenta_ident();
    Expr_multiplicativa();
    while (t.cat == SN && (t.codigo == SN_SOMA || t.codigo == SN_SUBTRACAO)) {
        print_folha(t); consome(SN, t.codigo);
        Expr_multiplicativa();
    }
    diminui_ident(); printf("%s</Expr_aditiva>\n", TABS);
}

void Expr_multiplicativa() {
    printf("%s<Expr_multiplicativa>\n", TABS); aumenta_ident();
    Fator();
    while (t.cat == SN && (t.codigo == SN_MULTIPLICACAO || t.codigo == SN_DIVISAO)) {
        print_folha(t); consome(SN, t.codigo);
        Fator();
    }
    diminui_ident(); printf("%s</Expr_multiplicativa>\n", TABS);
}

void Fator() {
    printf("%s<Fator>\n", TABS); aumenta_ident();
    if (t.cat == SN && (t.codigo == SN_SOMA || t.codigo == SN_SUBTRACAO || t.codigo == SN_NEGACAO)) {
        print_folha(t); consome(SN, t.codigo);
        Fator();
    } else if (t.cat == ID) {
        print_folha(t); consome(ID, 0);
        if (t.cat == SN && t.codigo == ABRE_COLCHETES) {
            print_folha(t); consome(SN, ABRE_COLCHETES);
            Expr();
            print_folha(t); consome(SN, FECHA_COLCHETES);
        } else if (t.cat == SN && t.codigo == ABRE_PARENTESES) {
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
    } else if (t.cat == CT_INT || t.cat == CT_REAL || t.cat == CT_CHAR) {
        print_folha(t); consome(t.cat, 0);
    } else if (t.cat == SN && t.codigo == ABRE_PARENTESES) {
        print_folha(t); consome(SN, ABRE_PARENTESES);
        Expr();
        print_folha(t); consome(SN, FECHA_PARENTESES);
    } else {
        error("Fator mal formado em uma expressao (esperado ID, constante ou '(' ).");
    }
    diminui_ident(); printf("%s</Fator>\n", TABS);
}