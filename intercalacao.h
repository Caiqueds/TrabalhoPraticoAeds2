#ifndef INTERCALACAO_H
#define INTERCALACAO_H

#include "lista.h"
#include "pilha.h"

/*estrutura para armazenar o vetor de partições*/
typedef struct vetor{
    TFunc *func;
    int aux_p;
    FILE *f;
}TVet;

/* Estrutura do Nó da Árvore de Vencedores */
typedef struct NoV {
    TFunc *vencedor;             // Aponta para o registro com a menor chave
    struct NoV *endVencedor;     // Aponta para o nó folha original do vencedor
    FILE *f;                     // Arquivo associado (usado apenas nas folhas)
    int aux_p;                   // Posicao (RRN) do proximo registro a ler no arquivo (folhas)
    struct NoV *pai;             // Ponteiro para o nó pai
    struct NoV *dir;             // Ponteiro para o filho da direita
    struct NoV *esq;             // Ponteiro para o filho da esquerda
} TNoV;


/* As duas funções retornam 1 em caso de sucesso ou 0 em caso de falha. */

/*Algoritmo de intercalacao de particoes básico, recebe
o nome do arquivo de saída, o número de partições a serem intercaladas
e uma lista com os nomes das partições de entrada a serem lidas.*/
int intercalacao_basico(char *nome_arquivo_saida, int num_p, Lista *nome_particoes);

/* Algoritmo de intercalacao de particoes usando arvore de vencedores a ser implementado no trabalho,
 fiquem a vontade para modificar a estrutura dos parâmetros, somente uma sugestão*/
// Algoritmo de intercalação usando árvore binária de vencedores


int intercalacao_arvore_vencedores(char *nome_arquivo_saida, int num_p, Lista *nome_particoes);

#endif
