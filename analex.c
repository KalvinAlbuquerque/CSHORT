#include "analex.h"


void error(char msg[]) 
{
    //fprintf(stderr, "Erro na linha %d: %s\n", contLinha, msg);
    printf("\nErro na linha %d: %s\n", contLinha, msg);
    exit(1); // Encerra o programa imediatamente em caso de erro léxico
}

bool is_letter(char c) 
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_digit(char c) 
{
    return (c >= '0' && c <= '9');
}

bool is_printable_ascii(char c) 
{
    return isprint(c);
}

int check_reserved_word(const char *lexema) 
{
    if (strcmp(lexema, "if") == 0)
    {
        return PR_IF;
    } 
    if (strcmp(lexema, "else") == 0)
    {
        return PR_ELSE;
    }
    if (strcmp(lexema, "while") == 0)
    {
        return PR_WHILE;
    }
    if (strcmp(lexema, "for") == 0)
    {
        return PR_FOR;
    } 
    if (strcmp(lexema, "return") == 0)
    {
        return PR_RETURN;
    } 
    if (strcmp(lexema, "intcon") == 0)
    {
        return PR_INTCON;
    } 
    if (strcmp(lexema, "realcon") == 0)
    {
        return PR_REALCON;
    }
    if (strcmp(lexema, "charcon") == 0)
    {
        return PR_CHARCON;
    } 
    if (strcmp(lexema, "stringcon") == 0)
    {
        return PR_STRINGCON;
    }
    if (strcmp(lexema, "break") == 0) 
    {
        return PR_BREAK;
    }
    if (strcmp(lexema, "continue") == 0)
    {
        return PR_CONTINUE;
    }
    if (strcmp(lexema, "void") == 0)
    {
        return PR_VOID;
    }
    if (strcmp(lexema, "bool") == 0)
    {
        return PR_BOOL;
    }
    if (strcmp(lexema, "float") == 0)
    {
        return PR_FLOAT;
    }
    if (strcmp(lexema, "double") == 0)
    {
        return PR_DOUBLE;
    }
    if (strcmp(lexema, "do") == 0)
    {
        return PR_DO;
    }

    return 0; // Não é palavra reservada
}

TOKEN Analex(FILE *fd) 
{
    int estado = 0; //Variável para controlar os estados do AFD
    char lexema[TAM_MAX_LEXEMA] = ""; //Array para guardar o lexema
    /* LEXEMA (Para nunca mais esquecer) 
        Definição: palavra ou símbolo específico encontrado no código-fonte
        que corresponde a uma categoria léxica(token), como palavra-chave,
        identificador, dígito, sinais, operador,etc.

        Token --> dígito
        Lexema --> 555555

        Token --> Categoria abstrata
        Lexema --> Representação concreta de um determinado token
    */
    int tamanho_lexema = 0; //Contador para armazenar o tamanho do lexema
    char digitos_int[TAM_NUM] = ""; //Array para armazenar os dígitos inteiros
    int tamanho_digito = 0; //Contador para armazenar o tamanho do dígito
    char digitos_real[TAM_NUM] = ""; //Array para armazenar os dígitos reais
    int tamanho_digito_real = 0; //Contador para armazenar o tamanho do digito real
    TOKEN t; //Variável token que será atualizada para cada token lido.
    int c; // 'c' DEVE SER INT para fgetc() retornar EOF corretamente

    /* 
        Loop principal. 
        Continua até encontrar um token ou um erro (neste caso, encerra o programa)
    */
    while (true) 
    {
        switch (estado) 
        {
            case 0: // Estado Inicial (Q0)

                /*  
                    Tratando espaços em branco e quebras de linha
                    Fica em um loop infinito até encontrar um caractere não branco ou um EOF
                */
                while (true) 
                { 
                    c = fgetc(fd);
                    //Se for vazio ou tab
                    if (c == ' ' || c == '\t' || c == '\r') 
                    {
                        continue; 
                    } 
                    else if (c == '\n') 
                    {
                        contLinha++;
                        continue; 
                    } 
                    else 
                    {
                        break; // Caractere não branco ou EOF encontrado, sai deste loop interno
                    }
                }

                // Se após consumir brancos, chegamos ao EOF, retorna o token FIM_ARQ.
                if (c == EOF) //Q28 no AFD
                {
                    t.cat = FIM_ARQ;
                    return t;
                }

                // Reseta os buffers e contadores para o novo token
                tamanho_lexema = 0;
                tamanho_digito = 0;
                tamanho_digito_real = 0;

                // Aqui 'c' é o primeiro caractere *significativo* de um potencial token.
                // Estado de Identificador ou Palavra Reservada (primeiro caractere é letra)
                if (is_letter(c)) 
                { 
                    estado = 1; //Vai para o estado de reconhecimento de Identificador/Palavra Reservada
                    if (tamanho_lexema < TAM_MAX_LEXEMA - 1) 
                    {
                        lexema[tamanho_lexema] = (char)c;
                        tamanho_lexema++;
                    }
                    else error("Identificador muito longo.");
                }
                // Início de Constante Inteira(primeiro caractere é dígito)
                else if (is_digit(c)) 
                { 
                    estado = 2; // Vai para o estado de reconhecimento de CT_INT 
                    if(tamanho_digito < TAM_NUM - 1) 
                    {
                        digitos_int[tamanho_digito] = (char)c;
                        tamanho_digito++;
                    }
                    else
                    {
                        error("Constante inteira muito longa.");
                    } 
                } 
                /* SINAIS/OPERADORES */
                /* Sinais que não precisam setar o estado, posso retornar logo aqui */
                //ADIÇÃO - Q23
                else if (c == '+') 
                {
                    t.cat = SN; 
                    t.codigo = SN_SOMA; 
                    return t;
                }
                //SUBTRAÇÃO - Q24
                else if (c == '-') 
                {
                    t.cat = SN; 
                    t.codigo = SN_SUBTRACAO;
                    return t;
                }
                //MULTIPLICAÇÃO - Q25
                else if (c == '*') 
                {
                    t.cat = SN; 
                    t.codigo = SN_MULTIPLICACAO; 
                    return t;
                }
                //ABRE PARÊNTESES  - Q18
                else if (c == '(') 
                {
                    t.cat = SN; 
                    t.codigo = ABRE_PARENTESES;
                    return t;
                }
                //FECHA PARÊNTESES - Q17 
                else if (c == ')') 
                {
                    t.cat = SN;
                    t.codigo = FECHA_PARENTESES; 
                    return t;
                }
                //ABRE CHAVES - Q19
                else if (c == '{') 
                {
                    t.cat = SN; 
                    t.codigo = ABRE_CHAVES; 
                    return t;
                }
                //FECHA CHAVES - Q20 
                else if (c == '}') 
                {
                    t.cat = SN; 
                    t.codigo = FECHA_CHAVES; 
                    return t;
                }
                //ABRE COLCHETE - Q21 
                else if (c == '[') 
                {
                    t.cat = SN; 
                    t.codigo = ABRE_COLCHETES; 
                    return t;
                }
                //FECHA COLCHETE - Q22 
                else if (c == ']') 
                {
                    t.cat = SN; 
                    t.codigo = FECHA_COLCHETES; 
                    return t;
                }
                //PONTO E VÍRGULA - Q26
                else if (c == ';') 
                { 
                    t.cat = SN;
                    t.codigo = PONTO_VIRGULA; 
                    return t;
                }
                //VÍRGULA - Q27
                else if (c == ',') 
                {
                    t.cat = SN; 
                    t.codigo = VIRGULA; 
                    return t;
                }
                //ATRIBUIÇÃO E POSSÍVEL COMPARAÇÃO - Q35 - Q37
                else if (c == '=') 
                {
                    estado = 8;
                }
                //NEGAÇÃO E POSSÍVEL DIFERENTE  - Q46 - Q48
                else if (c == '!') 
                {
                    estado = 9;
                } 
                //MENOR QUE E POSSÍVEL MENOR IGUAL - Q34 - Q33
                else if (c == '<') 
                {
                    estado = 10;
                } 
                //MAIOR QUE E POSSÍVEL MAIOR IGUAL - Q29 - Q32
                else if (c == '>') 
                {
                    estado = 11;
                } 
                // INÍCIO CHARCON - Q2 -Q5
                // Para não esquecer ... \' em C é a forma de você simbolizar as aspas simples ou apóstrofo
                else if (c == '\'') 
                {
                    estado = 12;                 
                }
                // INÍCIO STRINGCON - Q15-Q16
                else if (c == '"') 
                {
                    estado = 13; 
                }
                //Estado de divisão ou possível comentário
                //No AFD são Q42 - Q45
                else if (c == '/') 
                {
                    estado = 15; 
                } 
                //Estado inicial para && (operador AND)
                // No AFD são Q38 - Q39
                else if (c == '&') 
                { 
                    // Início potencial de &&
                    estado = 21;
                } 
                //Estado inicial para || (operador OR)
                // No AFD são Q40 - Q41
                else if (c == '|') 
                { 
                    estado = 22; //Vai para o estado do potencial OR
                }  
                //Se o caracter não for válido, aponto erro.
                else 
                {
                    // Se o caractere não se encaixar em NENHUMA regra léxica neste ponto, é um erro.
                    char msg_err[100]; 
                    sprintf(msg_err, "Caracter '%c' invalido.", c);
                    error(msg_err);
                }
                break;
            

            /************************************** TRATAMENTO DE ESTADOS *******************************************************/ 
            
            // Estado de Identificador ou Palavra Reservada (após a primeira letra)
            // No AFD é o Q1 - Q3
            case 1: 
                c = fgetc(fd); // Lê o próximo caractere
                //Se o próximo caractere 'c' for letra, digito ou '_'
                if (is_letter(c) || is_digit(c) || c == '_') 
                {   
                    //Se o lemexa for menor que o tamanho destinado para o nosso buffer (retirando o -1 do caractere final)
                    //adicionamos o 'c' lido ao lexema e incrementamos o contador de tamanho.
                    if (tamanho_lexema < TAM_MAX_LEXEMA - 1)
                    {
                        lexema[tamanho_lexema] = (char)c;
                        tamanho_lexema++;
                    }
                    else
                    {   
                        //Caso tenha excedido o tamanho do buffer estipulado
                        error("Identificador excede o tamanho maximo.");
                    }
                    //Permanece no estado 1.
                    //Não há troca de estado, pois aguardaremos mais letras, dígitos ou '_' para o Identificador ou Palavra Reservada.                      
                } 
                else 
                { 
                    // Fim do ID/Palavra Reservada
                    //Se o cactere 'c' não for letra, dígito ou '_', precisamos ir para outro estado.
                    ungetc(c, fd); // Devolvo 'c' para ser lido na próxima iteração
                    lexema[tamanho_lexema] = '\0'; // Termina o lexema incluindo o '\0' ao final.

                    //Verifico se é palavra reservada
                    int pr_codigo = check_reserved_word(lexema);
                    //Se for palavra reservada
                    if (pr_codigo != 0)
                    {
                        t.cat = PALAVRA_RESERVADA;
                        t.codigo = pr_codigo;
                    } 
                    //Se for Identificador
                    else 
                    {   
                        t.cat = ID;
                        strcpy(t.lexema, lexema);
                    }

                    //Retorno o token analisado
                    return t;
                }
                break;
            // Estado de Constante Inteira (após o primeiro dígito)
            // No AFD é o Q11 - Q12
            case 2: 
                c = fgetc(fd); // Lê o próximo caractere
                //Se o próximo caractere for dígito
                if (is_digit(c)) 
                {
                    //Se não estourar o buffer
                    if (tamanho_digito < TAM_NUM - 1)   
                    {
                        digitos_int[tamanho_digito] = (char)c;
                        tamanho_digito++;

                    }
                    else 
                    {
                        error("Constante inteira muito longa.");
                    }
                    
                    // Permanece no estado 2 
                    // Não há troca de estados, pois iremos ler todos os dígitos
                }
                //Se o caractere for um '.' é o começo de uma constante real, vamos ter que trocar de estado
                else if (c == '.') 
                { 
                    estado = 3; //Vai para o estado de CT_REAL
                    digitos_int[tamanho_digito] = '\0'; // Finaliza a parte inteira
                } 
                else 
                { 
                    // Fim da Constante Inteira
                    ungetc(c, fd); //Devolvo o dígito que não faz mais parte da CT_INT para que ele possa ser lido na próxima iteração
                    digitos_int[tamanho_digito] = '\0'; //Finalizo os digitos

                    t.cat = CT_INT; 
                    t.valInt = atoi(digitos_int); //Converto o valor para valor inteiro
                    return t; //retorno o token
                }
                break;
            // Estado de Constante Real - após o ponto '.'
            // No AFD é o Q13 
            case 3: 
                c = fgetc(fd); // Lê o próximo caractere
                // Se EOF após '.', é um erro
                if (c == EOF) 
                { 
                    error("Constante real mal formada: EOF apos o ponto.");
                }
                //Se for dígito 
                else if (is_digit(c)) 
                {
                    estado = 4; // Vai para o estado de Constante Real - dígitos após o ponto
                    if (tamanho_digito_real < TAM_NUM - 1) 
                    {
                        digitos_real[tamanho_digito_real] = (char)c;
                        tamanho_digito_real++;
                    }
                    else 
                    {
                        error("Parte decimal da constante real excede o tamanho maximo.");
                    }
                } 
                //Se o caractere depois do ponto não for dígito
                else 
                { 
                    error("Constante real mal formada: esperado digito apos o ponto.");
                }
                break;
            // Estado de Constante Real - dígitos após o ponto
            // No AFD é o Q14
            case 4: 
                c = fgetc(fd); // Lê o próximo caractere
                //Se 'c' for dígito
                if (is_digit(c)) 
                {
                    if (tamanho_digito_real < TAM_NUM - 1)
                    {
                        digitos_real[tamanho_digito_real++] = (char)c;
                    }
                    else
                    {
                        error("Parte decimal da constante real excede o tamanho maximo.");
                    }
                    // Permanece no estado 4 até que todos os dígitos sejam lidos (ou seja, até que um caractere diferente
                    //de dígito seja lido)
                } 
                else 
                {   
                    // Fim da Constante Real
                    ungetc(c, fd); //Devolve o caractere não dígito para ser lido na próxima iteração
                    digitos_real[tamanho_digito_real] = '\0'; //Finaliza o lexema dos dígitos reais

                    char num_completo[TAM_NUM * 2 + 2]; //Aumentando tamanho do número real completo, apenas por via das dúvidas (simular um float)
                    // Tamanho suficiente para int.real e '.' e '\0'
                    sprintf(num_completo, "%s.%s", digitos_int, digitos_real); //Concatenando todo o digito real: dígito para inteira + '.' + dígitos parte decimal
                    
                    t.cat = CT_REAL;
                    t.valReal = atof(num_completo); //Convertendo para float

                    return t; //retorno o token
                }
                break;
            // Estado de atribuição ou comparação (Após '=')
            // No AFD é o Q35 - Q37
            case 8:
                c = fgetc(fd); // Lê o próximo caractere
                //Se vier outra igualdade, é classificado como comparação e retorno o token de comparação
                if (c == '=') 
                { 
                    t.cat = SN;
                    t.codigo = SN_COMPARACAO; 
                    return t; 
                }
                //Atribuição
                else 
                { 
                    ungetc(c, fd); //Se o próximo caractere não for '=', então é uma atribuição. Devolvo o próximo caractere para próxima iteração.
                    t.cat = SN;
                    t.codigo = SN_ATRIBUICAO;
                    return t; 
                } 
                break;
            // Estado de negação ou diferença (Após '!')
            // No AFD é o Q46 - Q48
            case 9: 
                c = fgetc(fd); // Lê o próximo caractere
                //Se o próximo caractere for EOF, está incompleto, logo aponta erro
                if (c == EOF) 
                { 
                    error("Caracter invalido: '!' deve ser seguido por '=' (EOF atingido)."); 
                }
                //Se vier o '=', então é diferença
                else if (c == '=') 
                { 
                    t.cat = SN; 
                    t.codigo = SN_DIFERENTE; 
                    return t; 
                }
                //Se vier qualquer outro *, é negação.
                else
                { 
                    ungetc(c, fd); //Retorno o próximo caractere
                    t.cat = SN;
                    t.codigo = SN_NEGACAO;
                    return t;
                }
                break;
            // Estado menor que ou menor igual (Após '<')
            // No AFD é o Q34 - Q33
            case 10: 
                c = fgetc(fd); // Lê o próximo caractere
                //Se o próximo caractere for '=', então é menor igual (<=)
                if (c == '=') 
                { 
                    t.cat = SN;
                    t.codigo = SN_MENOR_IGUAL; 
                    return t; 
                }
                //Se for qualquer outro *, é menor que (<)
                else 
                { 
                    ungetc(c, fd); //Devolvo o caractere para a próxima iteração
                    t.cat = SN; 
                    t.codigo = SN_MENOR; 
                    return t; 
                }
                break;
            // Estado maior que ou maior igual (Após '>')
            // No AFD é o Q29 - Q32
            case 11: 
                c = fgetc(fd); // Lê o próximo caractere
                //Se o próximo caractere for '=', então é maior ou igual (>=)
                if (c == '=') 
                { 
                    t.cat = SN; 
                    t.codigo = SN_MAIOR_IGUAL; 
                    return t; 
                }
                //Se for qualquer outro *, é maior que ()>)
                else 
                { 
                    ungetc(c, fd); //Devolvo o caractere para o stream novamente
                    t.cat = SN; 
                    t.codigo = SN_MAIOR; 
                    return t; 
                }
                break;
            // Início Charcon (Após o primeiro apóstrofo ('))
            // No AFD é Q2 - 15
            case 12: 
                c = fgetc(fd); // Lê o caractere do charcon
                //Se o próximo caractere for um EOF, quer dizer que há um erro
                if (c == EOF) 
                { 
                    error("Constante de caractere nao fechada (EOF atingido)."); 
                }
                //Tratanto barra invertida
                //Se for barra invertida, vamos para outro estado tratar isso lá
                else if (c == '\\') //Psiu!! Para não esquecer: '\\' em c simboliza uma única barra invertida.
                { 
                    estado = 16; //Vai para o estado de tratamento de escape
                }
                /* 
                    SEGUNDO A ESPECIFICAÇÃO DA LINGUAGEM:
                    -charcon (constante de caractere): É definido como 'ch', onde ch denota qualquer caractere imprimível da tabela ASCII, 
                    diferente de \ (barra invertida) e ' (apóstrofo).
                    Ou seja, não permite aspas simples vazias (Mas permite strings vazias, como descrito também na especificação)

                    -stringcon (constante de string): É definido como "{ch}" onde ch denota qualquer caractere imprimível da tabela ASCII, diferente 
                    de " (aspas) e do caractere newline.
                    As chaves {a} na notação BNF estendida significam "repetição zero ou mais vezes". Portanto, {ch} significa que pode haver zero ou 
                    mais caracteres ch dentro das aspas duplas.
                    Isso implica que "" (string vazia) é permitido, pois é a ocorrência de zero caracteres ch.
                */
                else if (c == '\'') 
                { 
                    error("Aspas simples vazias não são permitidas.");
                } 
                //else if (c == '\n') 
                //{ 
                //   error("Constante de caractere contem quebra de linha nao escapada.");
                //   contLinha++; //incremento, pois o \n foi lido
                //}
                //Se for um caractere imprimível da tabela ascii
                else if (is_printable_ascii(c)) 
                { 
                    if (tamanho_lexema < TAM_MAX_LEXEMA - 1)
                    {
                        lexema[tamanho_lexema] = (char)c;
                        tamanho_lexema++;
                    } 
                    else 
                    {
                        error("Constante de caractere excede o tamanho maximo.");
                    }
                    estado = 17; // Estado de verificação de fechamento de apóstrofo
                } 
                else 
                {
                    char msg_err[100]; 
                    sprintf(msg_err, "Caracter '%c' invalido em constante de caractere.", c);
                    error(msg_err);
                }
                break;
            // Início estado de Stringcon (após primeiras aspas duplas '"')
            // No AFD são os estados Q15 - Q16
            case 13: 
                c = fgetc(fd); // Lê o caractere da string
                //Se for EOF, aponta erro
                if (c == EOF) 
                { 
                    error("Constante de string nao fechada (EOF atingido).");
                }
                //Se o próximo caractere for outra aspas, a stringcon acabou aqui
                /* 
                    -stringcon (constante de string): É definido como "{ch}" onde ch denota qualquer caractere imprimível da tabela ASCII, diferente 
                    de " (aspas) e do caractere newline.
                    As chaves {a} na notação BNF estendida significam "repetição zero ou mais vezes". Portanto, {ch} significa que pode haver zero ou 
                    mais caracteres ch dentro das aspas duplas.
                    Isso implica que "" (string vazia) é permitido, pois é a ocorrência de zero caracteres ch.
                 */
                else if (c == '"') 
                { 
                    // Fim da string
                    lexema[tamanho_lexema] = '\0'; //Finalizo o lexema
                    t.cat = CT_STRING;
                    strcpy(t.lexema, lexema);
                    return t;
                } 
                /* STRING E CHAR ME FODERAMMMMMMMMMMMMMMMMMMMM que porra 
                Depois vamos ver se realmente é necessário isso ai
                */
                /* 
                Conforme especificação... 
                stringcon ::=  "{ch}",  onde ch denota qualquer caractere imprimível da tabela ASCII, 
                como especificado pela função isprint() do C, diferente de  " (aspas) e do 
                caractere newline.
               
                Ou seja, " e \n devem ser tratadas de forma diferente
                */
                //Se houver uma barra invertida, vamos ter que tratar escapes para \n e para \" 
                else if (c == '\\') 
                { 
                    // Caractere de escape
                    estado = 18; //Estado para tratativa de caractere de escape
                } else if (c == '\n') { // Quebra de linha não escapada em Stringcon
                    error("Constante de string contem quebra de linha nao escapada.");
                }
                else if (is_printable_ascii(c) || c == '\r') { // Caractere normal ou retorno de carro
                    if (tamanho_lexema < TAM_MAX_LEXEMA - 1) lexema[tamanho_lexema++] = (char)c;
                    else error("Constante de string excede o tamanho maximo.");
                } else {
                    char msg_err[100]; // Tamanho aumentado
                    sprintf(msg_err, "Caracter '%c' invalido em constante de string.", c);
                    error(msg_err);
                }
                break;
            //Início de uma divisão ou possível comentário (Após "/")
            //No AFD são os estados Q42-Q45
            case 15: 
                c = fgetc(fd); // Lê o próximo caractere
                //Se for um asterístico, quer dizer que é início de comentário
                if (c == '*') 
                { 
                    estado = 19; //Vamos para o estado que trata o comentário
                } 
                //Se for qualquer outro * é divisão
                else 
                { 
                    ungetc(c, fd); //Devolvo o caractere para a próxima iteração 
                    t.cat = SN; 
                    t.codigo = SN_DIVISAO; 
                    return t; 
                }
                break;
            // Estado após barra invertida ('\') do charcon - tratativa de escapes 
            // No AFD são os estados Q6 - Q10
            case 16: 
                c = fgetc(fd); // Lê o caractere escapado
                //Se o próximo caractere for um EOF, aponto um erro
                if (c == EOF) 
                { 
                    error("Constante de caractere nao fechada: EOF apos escape. \\0 ou \\n"); 
                } 
                //Se o próximo caractere for um n, vai ser \n
                else if (c == 'n') 
                { 
                    //Se não estourar o buffer
                    if (tamanho_lexema < TAM_MAX_LEXEMA - 1) 
                    {
                        lexema[tamanho_lexema] = '\n'; //Adicione o \n no lexema
                        tamanho_lexema++; //Incrementa o contador
                        estado = 17; //Vai pro estado de verificação: se há um apóstrofo no final
                    }

                }
                //Se o próximo caractere for um 0, vai ser \0
                else if (c == '0') 
                { 
                    //Se não estourar o buffer
                    if (tamanho_lexema < TAM_MAX_LEXEMA - 1)
                    {
                        lexema[tamanho_lexema] = '\0'; //Adicione o \0 no lexema
                        tamanho_lexema++; //Incrementa o contadr
                        estado = 17; //Vai pro estado de verificaçaõ: se há um apóstrofo no final
                    }
                }
                //Se for qualquer outro *, eu aponto um erro. Pois só existem dois escapes \n e \0
                else 
                {
                    char msg_err[100]; 
                    sprintf(msg_err, "Sequencia de escape '\\%c' invalida.", c);
                    error(msg_err);
                }
                break;
            // Estado de fechamento de apóstrofo: foca em verificar se o charcon está fechado corretamente
            //Após o caractere ou escape, esperando a última aspa simples/apóstrofo
            case 17: 
                c = fgetc(fd); // Lê o caractere de fechamento
                //Se o próximo caractere for EOF, há um erro.
                if (c == EOF) 
                { 
                    error("Constante de caractere nao fechada (EOF atingido)."); 
                }
                //Se houver o apóstrofo de fechamento corretamente
                else if (c == '\'') 
                {
                    lexema[tamanho_lexema] = '\0'; // Finaliza o lexema

                    // Decide a categoria com base no *conteúdo* de lexema
                    //Se houver apenas um caractere dentro do lexema \0 ou \n
                    if (tamanho_lexema == 1) 
                    { 
                        //Se for \n
                        if (lexema[0] == '\n')
                        {
                            t.cat = CT_BN;
                        }
                        //Se for \0 
                        else if (lexema[0] == '\0') {
                            t.cat = CT_BZ;
                        } else {
                            t.cat = CT_CHAR;
                        }
                    } 
                    //Se houver mais de um caractere dentro do lexema
                    else 
                    {
                        error("Constante de caractere mal formada: mais de um caractere interno.");
                    }
                    strcpy(t.lexema, lexema); // Copia o caractere (ou '\n' ou '\0') para o lexema do token
                    return t;
                }
                //Se não houver apóstrofo, aponto um ero
                else 
                {
                    char msg_err[100]; 
                    sprintf(msg_err, "Constante de caractere mal formada: esperado 'c'.");
                    error(msg_err);
                }
                break;
            // Estado para tratativas de escapes do Stringcon (Após '\')
            // Os dois escapes são \n e \"
            case 18: 
                c = fgetc(fd); // Lê o próximo caractere 
                //Se o caractere é EOF, aponta erro
                if (c == EOF) 
                { 
                    error("Constante de string nao fechada: EOF apos escape.");
                } 
                //Se vier um n depois
                else if (c == 'n') 
                { 
                    if (tamanho_lexema < TAM_MAX_LEXEMA - 1)
                    {
                        lexema[tamanho_lexema] = '\n';
                        tamanho_lexema++;
                        estado = 13; //Volto pro estado de stringcon
                    }  
                }
                //Se vier uma aspas
                else if (c == '"') 
                { 
                    if (tamanho_lexema < TAM_MAX_LEXEMA - 1)
                    {
                        lexema[tamanho_lexema] = '\"'; 
                        tamanho_lexema++;
                        estado = 13; //Volto pro estado de stringcon
                    }  
                }
                //Se for qualquer outro caractere, aplicamos erros, pois não há nada na especificação que diga se devo aceitar outros escapes
                else 
                {
                    char msg_err[100]; 
                    sprintf(msg_err, "Sequencia de escape '\\%c' invalida em string.", c);
                    error(msg_err);
                }
                break;
            //Estado para tratar comentários
            //Após asterístico
            case 19: 
                // Dentro de comentário /*
                c = fgetc(fd); // Lê o caractere dentro do comentário
                //Se for EOF, aponta erro
                if (c == EOF) 
                { 
                    error("Comentario nao fechado (EOF atingido).");
                }
                //Se for outro asterístico, vamos pro estados para tratar o fechamento do comentário 
                else if (c == '*') 
                { 
                    estado = 20; //Estado para tratar o fechamento do comentário
                }
                //Se for qualquer outra coisa, permanece no mesmo estado, isto é, dentro do comentário
                //Só devemos consumir qualquer caractere, depois que o comentário for fechado. 
                //Portanto, devemos ficar atentos apenas para asterístico para irmos para o próximo estado 
                else 
                {
                    if (c == '\n')
                    {
                        contLinha++; //Incrementa linha, apenas para termos uma depuração precisa
                    }
                    // Permanece no estado 19
                }
                break;
            //Estado para tratar o fechamento do asterístico 
            //Estamos dentro do comentário e encontramos um asterístico (possívelmente o que antecede o fechamento do comentário)
            case 20: 
                c = fgetc(fd); // Lê o caractere de fechamento do comentário
                //Se for EOF, aponto erro
                if (c == EOF) 
                { 
                    error("Comentario nao fechado (EOF atingido).");
                }
                //Se for uma barra, finalizamos o comentário 
                else if (c == '/') 
                { 
                    // Fim de comentário */
                    estado = 0; // Comentário fechado, volta ao estado inicial para buscar o próximo token
                    // Não retorna token, o loop no `Analex continua do `case 0`.
                }
                //Se for outro caractere, eu volto pro estado 19 para tratativa 
                else 
                {    
                    ungetc(c, fd); // Devolve o caractere para ser reavaliado no estado 19
                    estado = 19; // Volta para o estado 19 (ainda dentro do comentário)
                }
                break;
            //Início para operador and (Após '&' (potencial '&&'))
            case 21: 
                c = fgetc(fd); // Lê o próximo caractere
                //Se for outro &, então encontramos o and
                if (c == '&') 
                {
                    t.cat = SN; 
                    t.codigo = SN_AND; 
                    return t;
                }
                //Se for qualquer outro caractere, aponta erro 
                else 
                {
                    error("Caracter invalido: '&' deve ser seguido por '&'.");
                }
                break;
            //Início para operador OR ( Após '|' (potencial '||'))
            case 22: 
                c = fgetc(fd); // Lê o próximo caractere
                //Se for outro |, encontramos nosso OR
                if (c == '|') 
                {
                    t.cat = SN; t.codigo = SN_OR; return t;
                }
                //Se for qualquer outro caracter, apontamos erro 
                else {
                    error("Caracter invalido: '|' deve ser seguido por '|'.");
                }
                break;
        }
    }
}

