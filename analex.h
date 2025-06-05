#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

/* CONSTANTES */
#ifndef ANALEX
#define ANALEX
#define TAM_MAX_LEXEMA 50
#define TAM_NUM 50

/* Variáveis Globais */
extern int contLinha; 

// Enum para as categorias de tokens
enum TOKEN_CAT 
{
    ID = 1,     //Identificador
    SN,         // Sinal/Operador
    CT_INT,     // Constante Intcon
    CT_REAL,    // Constante Realcon
    CT_CHAR,    // Constante de Charcon
    CT_STRING,  // Constante de Stringcon
    CT_BZ,      // Constante de escape /0
    CT_BN,      // Constante de escape /n
    COMENTARIO, // Comentário (embora não retorne token, pode ser útil para depuração)
    FIM_ARQ,    // Fim do arquivo
    PALAVRA_RESERVADA // Palavras reservadas 
};

// Enum para os códigos dos sinais/operadores
enum SINAIS 
{
    SN_SOMA = 1,
    SN_SUBTRACAO,
    SN_MULTIPLICACAO,
    SN_DIVISAO,
    SN_ATRIBUICAO,
    SN_MAIOR,
    SN_MENOR,
    SN_MAIOR_IGUAL,
    SN_MENOR_IGUAL,
    SN_COMPARACAO,
    SN_DIFERENTE,
    SN_NEGACAO,
    SN_AND,         
    SN_OR,          
    ABRE_PARENTESES,
    FECHA_PARENTESES,
    PONTO_VIRGULA,
    VIRGULA,
    ABRE_COLCHETES,
    FECHA_COLCHETES,
    ABRE_CHAVES,
    FECHA_CHAVES
};

// Enumeradores para as palavras reservadas (usar um enum separado ou códigos dentro de PALAVRA_RESERVADA)
enum PALAVRAS_RESERVADAS_CODIGO 
{
    PR_IF = 1,
    PR_ELSE,
    PR_WHILE,
    PR_FOR,
    PR_RETURN,
    PR_INTCON,
    PR_REALCON, 
    PR_CHARCON,
    PR_STRINGCON, 
    PR_BREAK,
    PR_CONTINUE,
    PR_VOID
};

typedef struct 
{
    enum TOKEN_CAT cat;     // Categoria do token
    union 
    {
        int codigo;         // Para tokens das categorias SN e PALAVRA_RESERVADA (usando o enum PALAVRAS_RESERVADAS_CODIGO)
        char lexema[TAM_MAX_LEXEMA]; // Para ID, CT_CHAR, CT_STRING (o valor literal)
        int valInt;         // Para CT_INT
        double valReal;     // Para CT_REAL
    };
} TOKEN;

/****************************** DECLARAÇÕES DE FUNÇÕES  ***********************************/

/* 
    Função para verificar se o caracter lido é uma letra  
    @param: char c--> o caractere a ser lido
    @return: bool --> um boleano indicando se é verdadeiro ou falso
*/
bool is_letter(char c);


/* 
    Função para verificar se o caracter lido é um dígito  
    @param: char c --> o caractere a ser lido
    @return: bool --> um boleano indicando se é verdadeiro ou falso 
*/
bool is_digit(char c);

/* 
    Função para verificar se o caracter é printável  
    @param: char c --> o caractere a ser lido
    @return: bool --> um boleano indicando se é verdadeiro ou falso
*/
bool is_printable_ascii(char c);

/* 
    Função para print de erro na análise léxica e parada de execução do programa.
    @param: char msg[] --> array contendo a mensagem a ser exibida
    @return: void
*/
void error(char msg[]);


/* 
    Função para verificar se o token é uma palavra reservada 
    @param: const char *lexema --> um vetor contendo o lexema a ser lido
    @return: int --> um dígito indicando o número daquela palavra reservada, caso seja verdadeiro
    Ou retorna 0 caso não seja palavra reservada.
*/
int check_reserved_word(const char *lexema);

/* 
    Função Analex é a mais importante do código
    Consome um arquivo e retorna o próximo token válido encontrado
    @param: FILE *fd --> um ponteiro apontando para o arquivo que será consumido
    @return: TOKEN --> o próximo token encontrado
*/
TOKEN Analex(FILE *fd);

#endif