#ifndef ANALEX
#define ANALEX
#define TAM_MAX_LEXEMA 50
#define TAM_NUM 50

// Enumeradores para as categorias de tokens
enum TOKEN_CAT {
    ID = 1,
    SN,         // Sinal/Operador
    CT_INT,     // Constante Intcon
    CT_REAL,    // Constante Realcon
    CT_CHAR,    // Constante de Charcon
    CT_STRING,  // Constante de Stringcon
    COMENTARIO, // Comentário (embora não retorne token, pode ser útil para depuração)
    FIM_EXPR,   // Representa o ';' (ponto e vírgula)
    FIM_ARQ,    // Fim do arquivo
    PALAVRA_RESERVADA // Palavras reservadas
};

// Enumeradores para os códigos dos sinais/operadores
enum SINAIS {
    OP_SOMA = 1,
    OP_SUBTRACAO,
    OP_MULTIPLICACAO,
    OP_DIVISAO,
    OP_ATRIBUICAO,
    OP_MAIOR,
    OP_MENOR,
    OP_MAIOR_IGUAL,
    OP_MENOR_IGUAL,
    OP_IGUAL,
    OP_DIFERENTE,
    OP_AND,         // Novo: Operador AND (&&)
    OP_OR,          // Novo: Operador OR (||)
    ABRE_PARENTESES,
    FECHA_PARENTESES,
    VIRGULA,
    ABRE_COLCHETES,
    FECHA_COLCHETES,
    ABRE_CHAVES,
    FECHA_CHAVES
};

// Enumeradores para as palavras reservadas (usar um enum separado ou códigos dentro de PALAVRA_RESERVADA)
enum PALAVRAS_RESERVADAS_CODIGO {
    PR_IF = 1,
    PR_ELSE,
    PR_WHILE,
    PR_FOR,
    PR_RETURN,
    PR_INT,
    PR_FLOAT, 
    PR_CHAR,
    PR_STRING, 
    PR_BREAK,
    PR_CONTINUE
};

typedef
    struct {
        enum TOKEN_CAT cat; // Categoria do token
        union {
            int codigo;         // Para tokens das categorias SN e PALAVRA_RESERVADA (usando o enum PALAVRAS_RESERVADAS_CODIGO)
            char lexema[TAM_MAX_LEXEMA]; // Para ID, CT_CHAR, CT_STRING (o valor literal)
            int valInt;         // Para CT_INT
            double valReal;     // Para CT_REAL
        };
    } TOKEN;

// Contador de linhas do código fonte (declarado como extern aqui, inicializado em .c)
extern int contLinha;

// Protótipo da função AnaLex
TOKEN AnaLex(FILE *fd);

#endif