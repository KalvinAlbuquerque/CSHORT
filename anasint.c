// Anasint.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>// Necessário para strcat e strlen
#include "anasint.h"
#include "analex.h"
#include "tabela_simbolos.h" // Inclusão do header da tabela de símbolos
#include "gerador_codigo.h"

// --- Variáveis Globais para Tabela de Símbolos ---
TokenInfo tokenInfo; // Estrutura para armazenar informações do token atual
Tabela tabela; // A tabela de símbolos

// --- Protótipos de Funções ---
void Prog();
void Decl_ou_Func();
void Func_body(int procPos);
void Decl();
void Decl_var_body();
void Decl_var();
int Tipo();
void Tipos_param();
void Cmd();
void Expr();
void Expr_atrib();
void Expr_ou();
void Expr_e();
void Expr_relacional();
void Expr_aditiva();
void Expr_multiplicativa();
void Fator();

/**
 * @brief Aumenta a indentação para a impressão da árvore sintática.
 */
void aumenta_ident() { if (strlen(TABS) < sizeof(TABS) - 3) strcat(TABS, " "); }

/**
 * @brief Diminui a indentação para a impressão da árvore sintática.
 */
void diminui_ident() { if (strlen(TABS) >= 2) TABS[strlen(TABS) - 2] = '\0'; }

/**
 * @brief Imprime um token formatado no console para fins de depuração.
 * @param tk O token a ser impresso.
 */
void print_folha(TOKEN tk) 
{
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

/**
 * @brief Converte um token (categoria e código) em uma descrição textual amigável.
 * @return Uma string constante (const char*) com a descrição do token.
 */
const char* getTokenDescription(int category, int code) {
    static char buffer[100];
    switch (category) {
        case ID: return "um Identificador";
        case CT_INT: return "uma constante inteira";
        case CT_REAL: return "uma constante real";
        case CT_CHAR: return "uma constante char";
        case CT_STRING: return "uma constante string";
        case FIM_ARQ: return "o fim do arquivo";
        case SN:
            switch (code) {
                case SN_SOMA: return "o operador '+'";
                case SN_SUBTRACAO: return "o operador '-'";
                case SN_MULTIPLICACAO: return "o operador '*'";
                case SN_DIVISAO: return "o operador '/'";
                case SN_ATRIBUICAO: return "o operador de atribuicao '='";
                case SN_MAIOR: return "o operador '>'";
                case SN_MENOR: return "o operador '<'";
                case SN_MAIOR_IGUAL: return "o operador '>='";
                case SN_MENOR_IGUAL: return "o operador '<='";
                case SN_COMPARACAO: return "o operador de comparacao '=='";
                case SN_DIFERENTE: return "o operador de diferenca '!='";
                case SN_NEGACAO: return "o operador de negacao '!'";
                case SN_AND: return "o operador logico '&&'";
                case SN_OR: return "o operador logico '||'";
                case ABRE_PARENTESES: return "o abre parenteses '('";
                case FECHA_PARENTESES: return "o fecha parenteses ')'";
                case PONTO_VIRGULA: return "o ponto e virgula ';'";
                case VIRGULA: return "a virgula ','";
                case ABRE_COLCHETES: return "o abre colchetes '['";
                case FECHA_COLCHETES: return "o fecha colchetes ']'";
                case ABRE_CHAVES: return "o abre chaves '{'";
                case FECHA_CHAVES: return "o fecha chaves '}'";
                default: return "um sinal/operador desconhecido";
            }
        case PALAVRA_RESERVADA:
            switch (code) {
                case PR_IF: return "a palavra reservada 'if'";
                case PR_ELSE: return "a palavra reservada 'else'";
                case PR_WHILE: return "a palavra reservada 'while'";
                case PR_FOR: return "a palavra reservada 'for'";
                case PR_RETURN: return "a palavra reservada 'return'";
                case PR_INTCON: return "o tipo 'int'";
                case PR_REALCON: return "o tipo 'real'";
                case PR_CHARCON: return "o tipo 'char'";
                case PR_STRINGCON: return "o tipo 'string'";
                case PR_VOID: return "o tipo 'void'";
                case PR_BOOL: return "o tipo 'bool'";
                case PR_FLOAT: return "o tipo 'float'";
                default: sprintf(buffer, "a palavra reservada de codigo %d", code); return buffer;
            }
        default: return "um token desconhecido";
    }
}

/**
 * @brief Verifica se o token atual é o esperado e avança para o próximo.
 * Se o token não for o esperado, formata e exibe um erro detalhado.
 */
void consome(int categoria_esperada, int codigo_esperado) {
    if (t.cat == categoria_esperada && (codigo_esperado == 0 || t.codigo == codigo_esperado)) {
        t = Analex(fd);
    } else {
        const char* desc_esperado = getTokenDescription(categoria_esperada, codigo_esperado);
        const char* desc_encontrado = getTokenDescription(t.cat, t.codigo);
        char msg_erro[256];

        if (t.cat == ID || t.cat == CT_STRING) {
            sprintf(msg_erro, "Token inesperado. Esperado %s, mas encontrado %s ('%s').", desc_esperado, desc_encontrado, t.lexema);
        } else if (t.cat == CT_INT) {
            sprintf(msg_erro, "Token inesperado. Esperado %s, mas encontrado %s (%d).", desc_esperado, desc_encontrado, t.valInt);
        } else if (t.cat == CT_REAL) {
            sprintf(msg_erro, "Token inesperado. Esperado %s, mas encontrado %s (%f).", desc_esperado, desc_encontrado, t.valReal);
        } else {
            sprintf(msg_erro, "Token inesperado. Esperado %s, mas encontrado %s.", desc_esperado, desc_encontrado);
        }
        error(msg_erro);
    }
}

/**
 * @brief Verifica se o token atual é uma palavra-chave de tipo.
 */
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

/**
 * @brief Ponto de entrada do analisador sintático. Analisa o programa inteiro.
 * Gramática: `prog ::= { decl ';' | func }`
 */
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
    limparTabela();
    diminui_ident(); printf("%s</Prog>\n", TABS);
}

/**
 * @brief Distingue entre uma declaração de variável e uma de função.
 */
void Decl_ou_Func() {
    printf("%s<Decl_ou_Func>\n", TABS); aumenta_ident();
    int tipo_atual = tokenInfo.tipo;
    if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_VOID) {
        tipo_atual = NA_TIPO;
    }
    
    print_folha(t); consome(t.cat, t.codigo);
    strcpy(tokenInfo.lexema, t.lexema);
    tokenInfo.tipo = tipo_atual;
    print_folha(t); consome(ID, 0);

    if (t.cat == SN && t.codigo == ABRE_PARENTESES) {
        tokenInfo.idcategoria = PROC;
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

/**
 * @brief Analisa o corpo e os parâmetros de uma função.
 * Gramática: `func ::= tipo id '(' tipos_param ')' '{' ... '}'`
 */
void Func_body(int procPos) {
    printf("%s<Func_body>\n", TABS); aumenta_ident();
    print_folha(t); consome(SN, ABRE_PARENTESES);
    tokenInfo.escopo = LOCAL;
    
    if (t.cat != SN || t.codigo != FECHA_PARENTESES) {
        Tipos_param();
    }
    
    print_folha(t); consome(SN, FECHA_PARENTESES);
    
    if (t.cat == SN && t.codigo == PONTO_VIRGULA) {
        print_folha(t); consome(SN, PONTO_VIRGULA);
    } else {
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

/**
 * @brief Analisa o restante de uma linha de declaração de variáveis.
 */
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

/**
 * @brief Analisa uma linha de declaração de variáveis locais.
 * Gramática: `decl ::= tipo decl_var { ',' decl_var } ';'`
 */
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

/**
 * @brief Analisa uma única declaração de variável (um `decl_var`).
 * Gramática: `decl_var ::= id [ '[' intcon ']' ]`
 */
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

/**
 * @brief Analisa a lista de parâmetros na declaração de uma função.
 * Gramática: `tipos_param ::= void | tipo (id | id '['']') { ',' ... }`
 */
void Tipos_param() {
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

/**
 * @brief Analisa um único comando da linguagem, aplicando as regras de tradução da MP.
 * Gramática: `cmd ::= if... | while... | for... | return... | atrib... | ...`
 */
void Cmd() 
{
    printf("%s<Cmd>\n", TABS); aumenta_ident();
    char linha[100]; // Buffer para gerar instruções

    if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_IF) {
        print_folha(t); consome(PALAVRA_RESERVADA, PR_IF);
        print_folha(t); consome(SN, ABRE_PARENTESES);

        Expr(); // Gera código para a condição do if

        print_folha(t); consome(SN, FECHA_PARENTESES);

        int rotulo_else = novo_rotulo();
        int rotulo_fim = novo_rotulo();

        // Se a condição for falsa (0), salta para o rótulo do else.
        sprintf(linha, "GOFALSE L%d", rotulo_else);
        gera(linha);

        Cmd(); // Corpo do if (bloco 'then')

        if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_ELSE) {
            // Se houver 'else', o bloco 'then' precisa saltar para o fim do if.
            sprintf(linha, "GOTO L%d", rotulo_fim);
            gera(linha);

            // Gera o rótulo para o início do bloco 'else'.
            sprintf(linha, "LABEL L%d", rotulo_else);
            gera(linha);

            print_folha(t); consome(PALAVRA_RESERVADA, PR_ELSE);
            Cmd(); // Corpo do else

            // Gera o rótulo para o fim da estrutura if-else.
            sprintf(linha, "LABEL L%d", rotulo_fim);
            gera(linha);
        } else {
            // Se não houver 'else', o 'GOFALSE' salta para este rótulo.
            sprintf(linha, "LABEL L%d", rotulo_else);
            gera(linha);
        }

    } else if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_WHILE) {
        print_folha(t); consome(PALAVRA_RESERVADA, PR_WHILE);
        print_folha(t); consome(SN, ABRE_PARENTESES);

        int rotulo_inicio = novo_rotulo();
        int rotulo_fim = novo_rotulo();

        // Gera o rótulo para o início do loop (teste da condição).
        sprintf(linha, "LABEL L%d", rotulo_inicio);
        gera(linha);

        Expr(); // Gera código para a condição

        print_folha(t); consome(SN, FECHA_PARENTESES);

        // Se a condição for falsa, salta para o fim do loop.
        sprintf(linha, "GOFALSE L%d", rotulo_fim);
        gera(linha);

        Cmd(); // Corpo do while

        // Salta de volta para o início para reavaliar a condição.
        sprintf(linha, "GOTO L%d", rotulo_inicio);
        gera(linha);

        // Gera o rótulo de fim do loop.
        sprintf(linha, "LABEL L%d", rotulo_fim);
        gera(linha);

    } else if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_FOR) {
        // NOTA: A estrutura deste parser para o 'for' é problemática.
        // Ele processa a parte de incremento (C) ANTES do corpo do loop (D)
        // em um laço for(A; B; C) { D; }. A ordem de execução correta seria A, B, D, C.
        // A ordem do parser leva a uma geração de código incorreta: A, B, C, D.
        // Para uma implementação correta, a estrutura do parser precisaria ser
        // modificada para adiar a geração de código da parte de incremento.
        // O código abaixo apenas analisa a sintaxe sem gerar código funcional.
        
        printf("%s- AVISO: A geracao de codigo para o laco 'for' nao foi implementada devido a uma limitacao estrutural do parser.\n", TABS);
        
        print_folha(t); consome(PALAVRA_RESERVADA, PR_FOR);
        print_folha(t); consome(SN, ABRE_PARENTESES);
        if (t.cat != SN || t.codigo != PONTO_VIRGULA) { Expr_atrib(); }
        print_folha(t); consome(SN, PONTO_VIRGULA);
        if (t.cat != SN || t.codigo != PONTO_VIRGULA) { Expr(); }
        print_folha(t); consome(SN, PONTO_VIRGULA);
        if (t.cat != SN || t.codigo != FECHA_PARENTESES) { Expr_atrib(); }
        print_folha(t); consome(SN, FECHA_PARENTESES);
        Cmd();

    } else if (t.cat == PALAVRA_RESERVADA && t.codigo == PR_RETURN) {
        print_folha(t); consome(PALAVRA_RESERVADA, PR_RETURN);
        if (t.cat != SN || t.codigo != PONTO_VIRGULA) {
            Expr(); // Gera código para a expressão de retorno (o valor fica no topo da pilha)
        }
        print_folha(t); consome(SN, PONTO_VIRGULA);
        gera("RET"); // Gera a instrução de retorno do procedimento

    } else if (t.cat == SN && t.codigo == ABRE_CHAVES) {
        print_folha(t); consome(SN, ABRE_CHAVES);
        while (!(t.cat == SN && t.codigo == FECHA_CHAVES)) {
            Cmd();
        }
        print_folha(t); consome(SN, FECHA_CHAVES);

    } else if (t.cat == SN && t.codigo == PONTO_VIRGULA) {
        // Comando vazio
        print_folha(t); consome(SN, PONTO_VIRGULA);

    } else {
        // Comando de expressão (ex: atribuição ou chamada de função)
        Expr();
        print_folha(t); consome(SN, PONTO_VIRGULA);
    }

    diminui_ident(); printf("%s</Cmd>\n", TABS);
}

/**
 * @brief Ponto de entrada para a análise de qualquer expressão.
 */
void Expr() {
    printf("%s<Expr>\n", TABS); aumenta_ident();
    Expr_atrib();
    diminui_ident(); printf("%s</Expr>\n", TABS);
}

/**
 * @brief Analisa uma expressão de atribuição.
 */
void Expr_atrib() {
    printf("%s<Expr_atrib>\n", TABS); aumenta_ident();
    Expr_ou();
    if (t.cat == SN && t.codigo == SN_ATRIBUICAO) {
        print_folha(t); consome(SN, SN_ATRIBUICAO);
        Expr_atrib();
        // A geração de código para atribuição (STOR) precisaria ser adicionada aqui
        // e dependeria de como o endereço da variável à esquerda é tratado.
    }
    diminui_ident(); printf("%s</Expr_atrib>\n", TABS);
}

/**
 * @brief Analisa expressões com o operador OU (||).
 */
void Expr_ou() {
    printf("%s<Expr_ou>\n", TABS); aumenta_ident();
    Expr_e();
    while (t.cat == SN && t.codigo == SN_OR) {
        print_folha(t); consome(SN, SN_OR);
        Expr_e();
        // Ação semântica para '||'
    }
    diminui_ident(); printf("%s</Expr_ou>\n", TABS);
}

/**
 * @brief Analisa expressões com o operador E (&&).
 */
void Expr_e() {
    printf("%s<Expr_e>\n", TABS); aumenta_ident();
    Expr_relacional();
    while (t.cat == SN && t.codigo == SN_AND) {
        print_folha(t); consome(SN, SN_AND);
        Expr_relacional();
        // Ação semântica para '&&'
    }
    diminui_ident(); printf("%s</Expr_e>\n", TABS);
}

/**
 * @brief Analisa expressões com operadores relacionais (==, !=, <, >, etc.).
 */
void Expr_relacional() {
    printf("%s<Expr_relacional>\n", TABS); aumenta_ident();
    Expr_aditiva();
    if (t.cat == SN && (t.codigo == SN_COMPARACAO || t.codigo == SN_DIFERENTE || t.codigo == SN_MAIOR || t.codigo == SN_MENOR || t.codigo == SN_MAIOR_IGUAL || t.codigo == SN_MENOR_IGUAL)) {
        int op = t.codigo;
        print_folha(t); consome(SN, t.codigo);
        Expr_aditiva();
        // Ação semântica para operadores relacionais (ex: SUB, seguido de teste)
    }
    diminui_ident(); printf("%s</Expr_relacional>\n", TABS);
}

/**
 * @brief Analisa expressões com operadores de adição e subtração (+, -).
 * Ação semântica: gera código 'ADD' ou 'SUB' após processar os dois operandos.
 */
void Expr_aditiva() {
    printf("%s<Expr_aditiva>\n", TABS); aumenta_ident();
    Expr_multiplicativa(); 
    while (t.cat == SN && (t.codigo == SN_SOMA || t.codigo == SN_SUBTRACAO)) {
        int op = t.codigo;
        print_folha(t); consome(SN, t.codigo);
        Expr_multiplicativa();

        if (op == SN_SOMA) {
            gera("ADD");
        } else {
            gera("SUB");
        }
    }
    diminui_ident(); printf("%s</Expr_aditiva>\n", TABS);
}

/**
 * @brief Analisa expressões com operadores de multiplicação e divisão (*, /).
 * Ação semântica: gera código 'MUL' ou 'DIV' após processar os dois operandos.
 */
void Expr_multiplicativa() {
    printf("%s<Expr_multiplicativa>\n", TABS); aumenta_ident();
    Fator();
    while (t.cat == SN && (t.codigo == SN_MULTIPLICACAO || t.codigo == SN_DIVISAO)) {
        int op = t.codigo;
        print_folha(t); consome(SN, t.codigo);
        Fator(); 

        if (op == SN_MULTIPLICACAO) {
            gera("MUL");
        } else {
            gera("DIV");
        }
    }
    diminui_ident(); printf("%s</Expr_multiplicativa>\n", TABS);
}

/**
 * @brief Analisa o menor componente de uma expressão (um "fator").
 * Ação semântica: gera código 'PUSH' para constantes, 'LOAD' para variáveis
 * e 'CALL' para funções.
 */
void Fator() {
    printf("%s<Fator>\n", TABS); aumenta_ident();
    char linha[100];

    if (t.cat == SN && (t.codigo == SN_SOMA || t.codigo == SN_SUBTRACAO || t.codigo == SN_NEGACAO)) {
        print_folha(t); consome(SN, t.codigo);
        Fator();
        // Adicionar geração de código para negação unária se necessário
    } else if (t.cat == ID) {
        char id_lexema[100];
        strcpy(id_lexema, t.lexema); // Salva o nome do identificador
        print_folha(t); consome(ID, 0);

        if (t.cat == SN && t.codigo == ABRE_PARENTESES) { // Chamada de função
            print_folha(t); consome(SN, ABRE_PARENTESES);
            if (!(t.cat == SN && t.codigo == FECHA_PARENTESES)) {
                Expr(); // Gera código para o primeiro argumento
                while (t.cat == SN && t.codigo == VIRGULA) {
                    print_folha(t); consome(SN, VIRGULA);
                    Expr(); // Gera código para os argumentos subsequentes
                }
            }
            print_folha(t); consome(SN, FECHA_PARENTESES);
            
            // Gera a instrução de chamada de procedimento
            // Assumindo que o rótulo da função é o próprio nome
            sprintf(linha, "CALL %s", id_lexema);
            gera(linha);

        } else { // Variável ou vetor
            if (t.cat == SN && t.codigo == ABRE_COLCHETES) { // Acesso a vetor
                print_folha(t); consome(SN, ABRE_COLCHETES);
                Expr(); // Gera código para a expressão do índice
                print_folha(t); consome(SN, FECHA_COLCHETES);
                // Geração de código para acesso a vetor (ex: ADD para calcular offset)
                // seria necessária aqui.
            }
            
            // Gera instrução para carregar o valor da variável na pilha.
            // Usando PUSH como substituto para LOAD m,n para simplicidade.
            sprintf(linha, "PUSH %s", id_lexema);
            gera(linha);
        }
    } else if (t.cat == CT_INT) {
        sprintf(linha, "PUSH %d", t.valInt);
        gera(linha);
        print_folha(t); consome(t.cat, 0);
    } else if (t.cat == CT_REAL) {
        sprintf(linha, "PUSH %f", t.valReal);
        gera(linha);
        print_folha(t); consome(t.cat, 0);
    } else if (t.cat == CT_CHAR) {
        sprintf(linha, "PUSH '%c'", t.valInt);
        gera(linha);
        print_folha(t); consome(t.cat, 0);
    } else if (t.cat == CT_STRING) {
        sprintf(linha, "PUSH \"%s\"", t.lexema);
        gera(linha);
        print_folha(t); consome(t.cat, 0);
    } else if (t.cat == SN && t.codigo == ABRE_PARENTESES) {
        print_folha(t); consome(SN, ABRE_PARENTESES);
        Expr();
        print_folha(t); consome(SN, FECHA_PARENTESES);
    } else {
        error("Fator mal formado. Esperado ID, constante ou '('");
    }
    diminui_ident(); printf("%s</Fator>\n", TABS);
}