#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "analex.h"

// Inicialização da variável contLinha
int contLinha = 1; // Inicia na linha 1

// Função para printar erro
void error(char msg[]) {
    printf("Erro na linha %d: %s\n", contLinha, msg);
    exit(1); // Encerra o programa imediatamente em caso de erro léxico
}

// Funções auxiliares...
bool is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_digit(char c) {
    return (c >= '0' && c <= '9');
}

bool is_printable_ascii(char c) {
    return isprint(c);
}

// Função para verificar se um lexema é uma palavra reservada
// Retorna o código da palavra reservada se for, 0 caso contrário
int check_reserved_word(const char *lexema) {
    if (strcmp(lexema, "if") == 0) return PR_IF;
    if (strcmp(lexema, "else") == 0) return PR_ELSE;
    if (strcmp(lexema, "while") == 0) return PR_WHILE;
    if (strcmp(lexema, "for") == 0) return PR_FOR;
    if (strcmp(lexema, "return") == 0) return PR_RETURN;
    if (strcmp(lexema, "int") == 0) return PR_INT;
    if (strcmp(lexema, "float") == 0) return PR_FLOAT;
    if (strcmp(lexema, "char") == 0) return PR_CHAR;
    if (strcmp(lexema, "string") == 0) return PR_STRING;
    if (strcmp(lexema, "break") == 0) return PR_BREAK;
    if (strcmp(lexema, "continue") == 0) return PR_CONTINUE;
    // Adicione mais palavras reservadas aqui
    return 0; // Não é palavra reservada
}


//Função AnaLex
//Recebe um arquivo
//Retorna um TOKEN
TOKEN AnaLex(FILE *fd) {
    int estado = 0;
    char lexema[TAM_MAX_LEXEMA] = "";
    int tamL = 0;
    char digitos_int[TAM_NUM] = "";
    int tamDI = 0;
    char digitos_real[TAM_NUM] = "";
    int tamDR = 0;
    TOKEN t;
    int c; // 'c' DEVE SER INT para fgetc() retornar EOF corretamente

    // Loop principal do analisador léxico que continua até encontrar um token
    // ou um erro fatal que encerre o programa.
    while (true) {
        switch (estado) {
            case 0: // Estado Inicial
                // --- TRATAMENTO DE ESPAÇOS EM BRANCO E QUEBRAS DE LINHA ---
                // Consome e ignora múltiplos espaços, tabulações, retornos de carro e quebras de linha.
                while (true) { // Loop interno para consumir brancos
                    c = fgetc(fd);
                    if (c == ' ' || c == '\t' || c == '\r') {
                        continue; // Continua consumindo espaços e tabulações
                    } else if (c == '\n') {
                        contLinha++;
                        continue; // Continua consumindo quebras de linha
                    } else {
                        break; // Caractere não branco ou EOF encontrado, sai deste loop interno
                    }
                }
                // --- Fim do tratamento de espaços ---

                // Se após consumir brancos, chegamos ao EOF, retorna o token FIM_ARQ.
                if (c == EOF) {
                    t.cat = FIM_ARQ;
                    return t;
                }

                // Reseta os buffers e contadores para o novo token
                tamL = 0;
                tamDI = 0;
                tamDR = 0;

                // Agora, 'c' é o primeiro caractere *significativo* de um potencial token.
                if (is_letter(c)) { // Início de Identificador ou Palavra Reservada
                    estado = 1; // Vai para o estado de reconhecimento de ID/Palavra Reservada
                    if (tamL < TAM_MAX_LEXEMA - 1) {
                        lexema[tamL++] = (char)c;
                    }
                    else error("Identificador muito longo.");
                } else if (is_digit(c)) { // Início de Constante Inteira (primeiro caractere é dígito)
                    estado = 2; // Vai para o estado de reconhecimento de CT_INT
                    if (tamDI < TAM_NUM - 1) digitos_int[tamDI++] = (char)c;
                    else error("Constante inteira muito longa.");
                } else if (c == '+') {
                    t.cat = SN; t.codigo = OP_SOMA; return t;
                } else if (c == '-') {
                    t.cat = SN; t.codigo = OP_SUBTRACAO; return t;
                } else if (c == '*') {
                    t.cat = SN; t.codigo = OP_MULTIPLICACAO; return t;
                } else if (c == '=') {
                    estado = 8;
                } else if (c == '!') {
                    estado = 9;
                } else if (c == '<') {
                    estado = 10;
                } else if (c == '>') {
                    estado = 11;
                } else if (c == '/') {
                    estado = 15; // Pode ser divisão ou início de comentário
                } else if (c == '&') { // Início potencial de &&
                    estado = 21;
                } else if (c == '|') { // Início potencial de ||
                    estado = 22;
                } else if (c == '(') {
                    t.cat = SN; t.codigo = ABRE_PARENTESES; return t;
                } else if (c == ')') {
                    t.cat = SN; t.codigo = FECHA_PARENTESES; return t;
                } else if (c == '{') {
                    t.cat = SN; t.codigo = ABRE_CHAVES; return t;
                } else if (c == '}') {
                    t.cat = SN; t.codigo = FECHA_CHAVES; return t;
                } else if (c == '[') {
                    t.cat = SN; t.codigo = ABRE_COLCHETES; return t;
                } else if (c == ']') {
                    t.cat = SN; t.codigo = FECHA_COLCHETES; return t;
                } else if (c == ';') { // ';' agora é FIM_EXPR
                    t.cat = FIM_EXPR; return t;
                } else if (c == ',') {
                    t.cat = SN; t.codigo = VIRGULA; return t;
                } else if (c == '\'') {
                    estado = 12; // Início de Charcon
                } else if (c == '"') {
                    estado = 13; // Início de Stringcon
                } else {
                    // Se o caractere não se encaixar em NENHUMA regra léxica neste ponto, é um erro.
                    char msg_err[100]; // Tamanho aumentado para evitar overflow
                    sprintf(msg_err, "Caracter '%c' invalido.", c);
                    error(msg_err);
                }
                break;

            case 1: // Estado de Identificador ou Palavra Reservada (após a primeira letra)
                c = fgetc(fd); // Lê o próximo caractere
                if (is_letter(c) || is_digit(c) || c == '_') {
                    if (tamL < TAM_MAX_LEXEMA - 1) lexema[tamL++] = (char)c;
                    else error("Identificador excede o tamanho maximo.");
                    // Permanece no estado 1, esperando mais caracteres do ID
                } else { // Fim do ID/Palavra Reservada - o caractere 'c' NÃO faz parte
                    ungetc(c, fd); // Devolve 'c' para ser lido na próxima iteração
                    lexema[tamL] = '\0'; // Termina o lexema
                    int pr_codigo = check_reserved_word(lexema); // Verifica se é palavra reservada

                    if (pr_codigo != 0) {
                        t.cat = PALAVRA_RESERVADA;
                        t.codigo = pr_codigo;
                    } else {
                        t.cat = ID;
                        strcpy(t.lexema, lexema);
                    }
                    estado = 0; // Resetar estado para o próximo token
                    return t;
                }
                break;

            case 2: // Estado de Constante Inteira (após o primeiro dígito)
                c = fgetc(fd); // Lê o próximo caractere
                if (is_digit(c)) {
                    if (tamDI < TAM_NUM - 1) digitos_int[tamDI++] = (char)c;
                    else error("Constante inteira muito longa.");
                    // Permanece no estado 2
                } else if (c == '.') { // Pode ser o início de uma constante real
                    estado = 3;
                    digitos_int[tamDI] = '\0'; // Finaliza a parte inteira
                } else { // Fim da Constante Inteira
                    ungetc(c, fd);
                    digitos_int[tamDI] = '\0';
                    t.cat = CT_INT;
                    t.valInt = atoi(digitos_int);
                    estado = 0;
                    return t;
                }
                break;

            case 3: // Estado de Constante Real - após o ponto
                c = fgetc(fd); // Lê o próximo caractere
                if (c == EOF) { // Se EOF após '.', é um erro
                    error("Constante real mal formada: EOF apos o ponto.");
                } else if (is_digit(c)) {
                    estado = 4;
                    if (tamDR < TAM_NUM - 1) digitos_real[tamDR++] = (char)c;
                    else error("Parte decimal da constante real excede o tamanho maximo.");
                } else { // Erro: Ponto sem dígitos decimais
                    error("Constante real mal formada: esperado digito apos o ponto.");
                }
                break;

            case 4: // Estado de Constante Real - dígitos após o ponto
                c = fgetc(fd); // Lê o próximo caractere
                if (is_digit(c)) {
                    if (tamDR < TAM_NUM - 1) digitos_real[tamDR++] = (char)c;
                    else error("Parte decimal da constante real excede o tamanho maximo.");
                    // Permanece no estado 4
                } else { // Fim da Constante Real
                    ungetc(c, fd);
                    digitos_real[tamDR] = '\0';
                    char num_completo[TAM_NUM * 2 + 2]; // Tamanho suficiente para int.real e '.' e '\0'
                    sprintf(num_completo, "%s.%s", digitos_int, digitos_real);
                    t.cat = CT_REAL;
                    t.valReal = atof(num_completo);
                    estado = 0;
                    return t;
                }
                break;

            case 8: // Após '=' (potencial '==' ou '=')
                c = fgetc(fd); // Lê o próximo caractere
                if (c == EOF) { ungetc(c, fd); t.cat = SN; t.codigo = OP_ATRIBUICAO; return t; } // Atribuição se EOF
                else if (c == '=') { t.cat = SN; t.codigo = OP_IGUAL; return t; } // COMPARAÇÃO (==)
                else { ungetc(c, fd); t.cat = SN; t.codigo = OP_ATRIBUICAO; return t; } // ATRIBUIÇÃO (=)
                break;

            case 9: // Após '!' (potencial '!=')
                c = fgetc(fd); // Lê o próximo caractere
                if (c == EOF) { error("Caracter invalido: '!' deve ser seguido por '=' (EOF atingido)."); }
                else if (c == '=') { t.cat = SN; t.codigo = OP_DIFERENTE; return t; }
                else { error("Caracter invalido: '!' deve ser seguido por '='."); }
                break;

            case 10: // Após '<' (potencial '<=' ou '<')
                c = fgetc(fd); // Lê o próximo caractere
                if (c == EOF) { ungetc(c, fd); t.cat = SN; t.codigo = OP_MENOR; return t; }
                else if (c == '=') { t.cat = SN; t.codigo = OP_MENOR_IGUAL; return t; }
                else { ungetc(c, fd); t.cat = SN; t.codigo = OP_MENOR; return t; }
                break;

            case 11: // Após '>' (potencial '>=' ou '>')
                c = fgetc(fd); // Lê o próximo caractere
                if (c == EOF) { ungetc(c, fd); t.cat = SN; t.codigo = OP_MAIOR; return t; }
                else if (c == '=') { t.cat = SN; t.codigo = OP_MAIOR_IGUAL; return t; }
                else { ungetc(c, fd); t.cat = SN; t.codigo = OP_MAIOR; return t; }
                break;

            case 12: // Início de Charcon (após o primeiro '\'')
                c = fgetc(fd); // Lê o caractere do charcon
                if (c == EOF) { error("Constante de caractere nao fechada (EOF atingido)."); }
                else if (c == '\\') { // É um caractere de escape
                    estado = 16;
                } else if (c == '\'') { // Aspa simples vazia, não é um char válido (e.g. '')
                    error("Constante de caractere vazia ou mal formada.");
                } else if (c == '\n') { // Quebra de linha não escapada em Charcon
                    error("Constante de caractere contem quebra de linha nao escapada.");
                }
                else if (is_printable_ascii(c)) { // Caractere imprimível comum
                    if (tamL < TAM_MAX_LEXEMA - 1) lexema[tamL++] = (char)c;
                    else error("Constante de caractere excede o tamanho maximo.");
                    estado = 17; // Espera a aspa de fechamento
                } else {
                    char msg_err[100]; // Tamanho aumentado para evitar overflow
                    sprintf(msg_err, "Caracter '%c' invalido em constante de caractere.", c);
                    error(msg_err);
                }
                break;

            case 16: // Após '\' em Charcon (tratando caractere escapado)
                c = fgetc(fd); // Lê o caractere escapado
                if (c == EOF) { error("Constante de caractere nao fechada: EOF apos escape."); } // EOF aqui também é erro
                else if (c == 'n') { if (tamL < TAM_MAX_LEXEMA - 1) lexema[tamL++] = '\n'; estado = 17; }
                else if (c == 't') { if (tamL < TAM_MAX_LEXEMA - 1) lexema[tamL++] = '\t'; estado = 17; }
                else if (c == '\\') { if (tamL < TAM_MAX_LEXEMA - 1) lexema[tamL++] = '\\'; estado = 17; }
                else if (c == '\'') { if (tamL < TAM_MAX_LEXEMA - 1) lexema[tamL++] = '\''; estado = 17; }
                else if (c == '0') { if (tamL < TAM_MAX_LEXEMA - 1) lexema[tamL++] = '\0'; estado = 17; }
                else {
                    char msg_err[100]; // Tamanho aumentado
                    sprintf(msg_err, "Sequencia de escape '\\%c' invalida.", c);
                    error(msg_err);
                }
                break;

            case 17: // Após o caractere do Charcon, esperando '
                c = fgetc(fd); // Lê o caractere de fechamento
                if (c == EOF) { error("Constante de caractere nao fechada (EOF atingido)."); }
                else if (c == '\'') {
                    lexema[tamL] = '\0';
                    t.cat = CT_CHAR;
                    strcpy(t.lexema, lexema);
                    estado = 0; // Volta ao estado inicial
                    return t;
                } else {
                    char msg_err[100]; // Tamanho aumentado
                    sprintf(msg_err, "Constante de caractere mal formada: esperado ''' mas leu '%c'.", c);
                    error(msg_err);
                }
                break;

            case 13: // Início de Stringcon (após o primeiro '"')
                c = fgetc(fd); // Lê o caractere da string
                if (c == EOF) { error("Constante de string nao fechada (EOF atingido)."); }
                else if (c == '"') { // Fim da string
                    lexema[tamL] = '\0';
                    t.cat = CT_STRING;
                    strcpy(t.lexema, lexema);
                    estado = 0; // Volta ao estado inicial
                    return t;
                } else if (c == '\\') { // Caractere de escape
                    estado = 18;
                } else if (c == '\n') { // Quebra de linha não escapada em Stringcon
                    error("Constante de string contem quebra de linha nao escapada.");
                }
                else if (is_printable_ascii(c) || c == '\r') { // Caractere normal ou retorno de carro
                    if (tamL < TAM_MAX_LEXEMA - 1) lexema[tamL++] = (char)c;
                    else error("Constante de string excede o tamanho maximo.");
                } else {
                    char msg_err[100]; // Tamanho aumentado
                    sprintf(msg_err, "Caracter '%c' invalido em constante de string.", c);
                    error(msg_err);
                }
                break;

            case 18: // Após '\' em Stringcon (tratando caractere escapado)
                c = fgetc(fd); // Lê o caractere escapado
                if (c == EOF) { error("Constante de string nao fechada: EOF apos escape."); } // EOF aqui também é erro
                else if (c == 'n') { if (tamL < TAM_MAX_LEXEMA - 1) lexema[tamL++] = '\n'; estado = 13; }
                else if (c == 't') { if (tamL < TAM_MAX_LEXEMA - 1) lexema[tamL++] = '\t'; estado = 13; }
                else if (c == '\\') { if (tamL < TAM_MAX_LEXEMA - 1) lexema[tamL++] = '\\'; estado = 13; }
                else if (c == '"') { if (tamL < TAM_MAX_LEXEMA - 1) lexema[tamL++] = '\"'; estado = 13; }
                else {
                    char msg_err[100]; // Tamanho aumentado
                    sprintf(msg_err, "Sequencia de escape '\\%c' invalida em string.", c);
                    error(msg_err);
                }
                break;

            case 15: // Após '/' (potencial divisão ou início de comentário)
                c = fgetc(fd); // Lê o próximo caractere
                if (c == EOF) { ungetc(c, fd); t.cat = SN; t.codigo = OP_DIVISAO; return t; } // Divisão se EOF
                else if (c == '*') { estado = 19; } // Início de comentário /*
                else { ungetc(c, fd); t.cat = SN; t.codigo = OP_DIVISAO; return t; } // Não é comentário, é operador de divisão
                break;

            case 19: // Dentro de comentário /*
                c = fgetc(fd); // Lê o caractere dentro do comentário
                if (c == EOF) { // Se atingir EOF enquanto dentro do comentário
                    error("Comentario nao fechado (EOF atingido).");
                } else if (c == '*') { // Potencial fim de comentário
                    estado = 20;
                } else {
                    if (c == '\n') contLinha++; // Incrementa linha dentro do comentário
                    // Permanece no estado 19
                }
                break;

            case 20: // Após '*' dentro de comentário, esperando '/'
                c = fgetc(fd); // Lê o caractere de fechamento do comentário
                if (c == EOF) { // Se atingir EOF aqui
                    error("Comentario nao fechado (EOF atingido).");
                } else if (c == '/') { // Fim de comentário */
                    estado = 0; // Comentário fechado, volta ao estado inicial para buscar o próximo token
                    // Não retorna token, o loop `while(true)` no `AnaLex` continua do `case 0`.
                } else { // Não era o fim do comentário (e.g., "**texto**")
                    ungetc(c, fd); // Devolve o caractere para ser reavaliado no estado 19
                    estado = 19; // Volta para o estado 19 (ainda dentro do comentário)
                }
                break;

            case 21: // Após '&' (potencial '&&')
                c = fgetc(fd); // Lê o próximo caractere
                if (c == '&') {
                    t.cat = SN; t.codigo = OP_AND; return t;
                } else {
                    error("Caracter invalido: '&' deve ser seguido por '&'.");
                }
                break;

            case 22: // Após '|' (potencial '||')
                c = fgetc(fd); // Lê o próximo caractere
                if (c == '|') {
                    t.cat = SN; t.codigo = OP_OR; return t;
                } else {
                    error("Caracter invalido: '|' deve ser seguido por '|'.");
                }
                break;
        }
    }
}

int main() {
    FILE *fd;
    TOKEN tk;
    // last_token_line_printed armazena a última linha cujo prefixo "LINHA X:" foi impresso.
    // Inicializa com 0 para garantir que a primeira linha seja impressa.
    int last_token_line_printed = 0;

    if ((fd = fopen("programa_cshort.txt", "r")) == NULL) {
        // Se o arquivo não puder ser aberto, a função error() em analex.c será chamada e o programa encerrado.
        error("Arquivo de entrada do programa nao encontrado! Crie 'programa_cshort.txt'");
    }

    printf("Iniciando analise lexica (modo rigido)...\n");
    printf("-------------------------------------------\n");

    while (true) {
        tk = AnaLex(fd);

        // Lógica de impressão do prefixo da linha:
        // Imprime "LINHA X: " apenas quando a linha atual (contLinha) é diferente
        // da última linha para a qual o prefixo foi impresso.
        if (contLinha != last_token_line_printed) {
            // Se não é a primeira linha a ser impressa e a anterior não terminou com FIM_EXPR (que já imprime \n)
            // Ou se a última linha impressa foi 0 (significa que é a primeira linha a ser impressa)
            if (last_token_line_printed != 0) {
                 printf("\n"); // Adiciona uma nova linha antes de imprimir um novo prefixo
            }
            printf("LINHA %d: ", contLinha);
            last_token_line_printed = contLinha;
        }

        switch (tk.cat) {
            case ID:
                printf("<ID, %s> ", tk.lexema);
                break;
            case SN:
                switch (tk.codigo) {
                    case OP_SOMA: printf("<SN, OP_SOMA> "); break;
                    case OP_SUBTRACAO: printf("<SN, OP_SUBTRACAO> "); break;
                    case OP_MULTIPLICACAO: printf("<SN, OP_MULTIPLICACAO> "); break;
                    case OP_DIVISAO: printf("<SN, OP_DIVISAO> "); break;
                    case OP_ATRIBUICAO: printf("<SN, OP_ATRIBUICAO> "); break;
                    case OP_MAIOR: printf("<SN, OP_MAIOR> "); break;
                    case OP_MENOR: printf("<SN, OP_MENOR> "); break;
                    case OP_MAIOR_IGUAL: printf("<SN, OP_MAIOR_IGUAL> "); break;
                    case OP_MENOR_IGUAL: printf("<SN, OP_MENOR_IGUAL> "); break;
                    case OP_IGUAL: printf("<SN, OP_IGUAL> "); break;
                    case OP_DIFERENTE: printf("<SN, OP_DIFERENTE> "); break;
                    case OP_AND: printf("<SN, OP_AND> "); break; // Novo
                    case OP_OR: printf("<SN, OP_OR> "); break;   // Novo
                    case ABRE_PARENTESES: printf("<SN, ABRE_PARENTESES> "); break;
                    case FECHA_PARENTESES: printf("<SN, FECHA_PARENTESES> "); break;
                    case VIRGULA: printf("<SN, VIRGULA> "); break;
                    case ABRE_COLCHETES: printf("<SN, ABRE_COLCHETES> "); break;
                    case FECHA_COLCHETES: printf("<SN, FECHA_COLCHETES> "); break;
                    case ABRE_CHAVES: printf("<SN, ABRE_CHAVES> "); break;
                    case FECHA_CHAVES: printf("<SN, FECHA_CHAVES> "); break;
                }
                break;
            case CT_INT:
                printf("<CT_INT, %d> ", tk.valInt);
                break;
            case CT_REAL:
                printf("<CT_REAL, %lf> ", tk.valReal);
                break;
            case CT_CHAR:
                printf("<CT_CHAR, '%s'> ", tk.lexema);
                break;
            case CT_STRING:
                printf("<CT_STRING, \"%s\"> ", tk.lexema);
                break;
            case PALAVRA_RESERVADA: // Novo case para palavras reservadas
                switch (tk.codigo) {
                    case PR_IF: printf("<PR, if> "); break;
                    case PR_ELSE: printf("<PR, else> "); break;
                    case PR_WHILE: printf("<PR, while> "); break;
                    case PR_FOR: printf("<PR, for> "); break;
                    case PR_RETURN: printf("<PR, return> "); break;
                    case PR_INT: printf("<PR, int> "); break;
                    case PR_FLOAT: printf("<PR, float> "); break;
                    case PR_CHAR: printf("<PR, char> "); break;
                    case PR_STRING: printf("<PR, string> "); break;
                    case PR_BREAK: printf("<PR, break> "); break;
                    case PR_CONTINUE: printf("<PR, continue> "); break;
                    // Adicione mais casos para palavras reservadas aqui
                }
                break;

            case FIM_EXPR: // É o ';'
                printf("<FIM_EXPR, ;>\n"); // Sempre termina com nova linha e o ';'
                break;
            case FIM_ARQ:
                printf("<FIM_ARQ>\n");
                break;
            default:
                break;
        }
        if (tk.cat == FIM_ARQ) break; // Sai do loop principal se o token FIM_ARQ for encontrado
    }

    printf("-------------------------------------------\n");
    printf("Analise lexica concluida.\n");
    fclose(fd);
    return 0;
}