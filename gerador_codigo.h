#ifndef GERADOR_CODIGO_H
#define GERADOR_CODIGO_H

#include <stdio.h>

/**
 * @brief Abre o arquivo de saída para a escrita do código da Máquina de Pilha.
 * @param nome_arquivo O nome do arquivo a ser criado (ex: "saida.txt").
 */
void inicializar_gerador(const char* nome_arquivo);

/**
 * @brief Fecha o arquivo de saída.
 */
void finalizar_gerador();

/**
 * @brief Escreve uma instrução formatada no arquivo de saída.
 * Funciona de forma similar ao printf.
 * @param formato A string de formato da instrução.
 * @param ... Argumentos variáveis para o formato.
 */
void gerar_codigo(const char* formato, ...);

/**
 * @brief Gera um novo rótulo único (L0, L1, L2, ...).
 * @param rotulo Um buffer de char para armazenar o rótulo gerado.
 */
void novo_rotulo(char* rotulo);

#endif // GERADOR_CODIGO_H