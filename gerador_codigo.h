#ifndef GERADOR_CODIGO_H
#define GERADOR_CODIGO_H

// Gera uma instrução de máquina de pilha com um parâmetro (pode ser vazio).
void gera(char *instrucao);

// Retorna um número de rótulo único para os desvios (JMP, JMP_FALSE).
int novo_rotulo();

void gera_rotulo(int r);

void salvar_codigo_em_arquivo(const char *nome_arquivo);

#endif // GERADOR_CODIGO_H