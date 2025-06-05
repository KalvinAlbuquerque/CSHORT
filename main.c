#include "analex.h"

int main() 
{
    FILE *fd;
    TOKEN tk;
    int linha_do_ultimo_token_impresso = 0;
    if ((fd = fopen("programa_cshort.txt", "r")) == NULL) 
    {
        error("Arquivo de entrada do programa nao encontrado! Crie 'programa_cshort.txt'");
    }

    printf("Iniciando analise lexica...\n");
    printf("-------------------------------------------\n");
    
    while (true) 
    {
        tk = Analex(fd);

        if (contLinha != linha_do_ultimo_token_impresso) 
        {
            printf("\nLINHA %d: ", contLinha);
            linha_do_ultimo_token_impresso = contLinha;
        }

        switch (tk.cat) 
        {
            case ID:
                printf("<ID, %s> ", tk.lexema);
                break;
            case SN:
                switch (tk.codigo) 
                {
                    case SN_SOMA: printf("<SN, SN_SOMA> "); break;
                    case SN_SUBTRACAO: printf("<SN, SN_SUBTRACAO> "); break;
                    case SN_MULTIPLICACAO: printf("<SN, SN_MULTIPLICACAO> "); break;
                    case SN_DIVISAO: printf("<SN, SN_DIVISAO> "); break;
                    case SN_MAIOR: printf("<SN, SN_MAIOR> "); break;
                    case SN_MENOR: printf("<SN, SN_MENOR> "); break;
                    case SN_MAIOR_IGUAL: printf("<SN, SN_MAIOR_IGUAL> "); break;
                    case SN_MENOR_IGUAL: printf("<SN, SN_MENOR_IGUAL> "); break;
                    case SN_ATRIBUICAO: printf("<SN, SN_ATRIBUICAO> "); break;
                    case SN_COMPARACAO: printf("<SN, SN_COMPARACAO> "); break;
                    case SN_DIFERENTE: printf("<SN, SN_DIFERENTE> "); break;
                    case SN_NEGACAO: printf("<SN, SN_NEGACAO>"); break;
                    case SN_AND: printf("<SN, SN_AND> "); break; 
                    case SN_OR: printf("<SN, SN_OR> "); break;   
                    case ABRE_PARENTESES: printf("<SN, ABRE_PARENTESES> "); break;
                    case FECHA_PARENTESES: printf("<SN, FECHA_PARENTESES> "); break;
                    case VIRGULA: printf("<SN, VIRGULA> "); break;
                    case ABRE_COLCHETES: printf("<SN, ABRE_COLCHETES> "); break;
                    case FECHA_COLCHETES: printf("<SN, FECHA_COLCHETES> "); break;
                    case ABRE_CHAVES: printf("<SN, ABRE_CHAVES> "); break;
                    case FECHA_CHAVES: printf("<SN, FECHA_CHAVES> "); break;
                    case PONTO_VIRGULA: printf("<SN, PONTO_VIRGULA>"); break;
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
            case CT_BN:
                printf("<CT_BN, \\n>");
                break;
            case CT_BZ:
                printf("<CT_BZ, \\0>");
                break;
            case PALAVRA_RESERVADA: 
                switch (tk.codigo) {
                    case PR_IF: printf("<PR, if> "); break;
                    case PR_ELSE: printf("<PR, else> "); break;
                    case PR_WHILE: printf("<PR, while> "); break;
                    case PR_FOR: printf("<PR, for> "); break;
                    case PR_RETURN: printf("<PR, return> "); break;
                    case PR_INTCON: printf("<PR, intcon> "); break;
                    case PR_REALCON: printf("<PR, realcon> "); break;
                    case PR_CHARCON: printf("<PR, charcon> "); break;
                    case PR_STRINGCON: printf("<PR, stringcon> "); break;
                    case PR_BREAK: printf("<PR, break> "); break;
                    case PR_CONTINUE: printf("<PR, continue> "); break;
                    case PR_VOID: printf("<PR, void> "); break;
                    case PR_FLOAT: printf("<PR, float> "); break;
                    case PR_DO: printf("<PR, do> "); break;
                    case PR_DOUBLE: printf("<PR, double> "); break;
                    case PR_BOOL: printf("<PR, bool> "); break;

                }
                break;
            case FIM_ARQ:
                printf("<FIM_ARQ>\n");
                printf("-------------------------------------------\n");
                printf("Analise lexica concluida.\n");
                fclose(fd);
                return 0;
                break;
            default:
                break;
        }
    }

    return 0;
}