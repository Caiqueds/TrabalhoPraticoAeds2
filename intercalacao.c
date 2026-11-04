#include "intercalacao.h"
#include "funcionarios.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int intercalacao_basico(char *nome_arquivo_saida, int num_p, Lista *nome_particoes) {
    if (num_p <= 0) return 0;
    TVet *v = calloc(num_p, sizeof(*v));
    FILE *out = NULL;
    int sucesso = 0;
    if (v == NULL) {
        printf("Não foi possível alocar memória para a intercalação.\n");
        return 0;
    }

    for (int i = 0; i < num_p; i++, nome_particoes = nome_particoes->prox) {
        if (nome_particoes == NULL) goto fim;
        v[i].f = fopen(nome_particoes->nome, "rb");
        if (v[i].f == NULL) {
            printf("Não foi possível abrir a partição %s.\n", nome_particoes->nome);
            goto fim;
        }
        v[i].func = le_funcionario(v[i].f);
        if (ferror(v[i].f)) goto fim;
        v[i].aux_p = 1;
    }

    out = fopen(nome_arquivo_saida, "wb");
    if (out == NULL) goto fim;

    for (int pos_saida = 0; ; pos_saida++) {
        int menor = -1;
        for (int i = 0; i < num_p; i++) {
            if (v[i].func != NULL && (menor < 0 || v[i].func->cod < v[menor].func->cod))
                menor = i;
        }
        if (menor < 0) break;

        if (fseek(out, (long)pos_saida * tamanho_registro(), SEEK_SET) != 0) goto fim;
        salva_funcionario(v[menor].func, out);
        if (ferror(out)) goto fim;

        if (fseek(v[menor].f, (long)v[menor].aux_p * tamanho_registro(), SEEK_SET) != 0) goto fim;
        free(v[menor].func);
        v[menor].func = le_funcionario(v[menor].f);
        if (ferror(v[menor].f)) goto fim;
        v[menor].aux_p++;
    }
    sucesso = 1;

fim:
    for (int i = 0; i < num_p; i++) {
        if (v[i].f != NULL) fclose(v[i].f);
        free(v[i].func);
    }
    free(v);
    if (out != NULL) {
        if (fclose(out) != 0) sucesso = 0;
        if (!sucesso) remove(nome_arquivo_saida);
    }
    if (!sucesso) printf("A intercalação básica não foi concluída.\n");
    return sucesso;
}

// Função auxiliar para criar os nós folha da árvore
static TNoV* cria_no_v(TFunc *func, FILE *f) {
    TNoV *novo = (TNoV*) malloc(sizeof(TNoV));
    if (novo == NULL) return NULL;
    novo->vencedor = func;
    novo->endVencedor = novo; // A folha aponta para si mesma como origem
    novo->f = f;
    novo->aux_p = 1; // o registro 0 ja foi lido na inicializacao
    novo->pai = NULL;
    novo->esq = NULL;
    novo->dir = NULL;
    return novo;
}



/* Propaga o menor registro entre os dois filhos, ignorando folhas esgotadas. */
static void atualiza_vencedor(TNoV *no) {
    TNoV *esq = no->esq;
    TNoV *dir = no->dir;
    TNoV *menor = esq;
    if (esq->vencedor == NULL ||
        (dir->vencedor != NULL && dir->vencedor->cod < esq->vencedor->cod)) {
        menor = dir;
    }
    no->vencedor = menor->vencedor;
    no->endVencedor = menor->endVencedor;
}

int intercalacao_arvore_vencedores(char *nome_arquivo_saida, int num_p, Lista *nome_particoes) {
    if (num_p <= 0 || num_p > INT_MAX / 2) {
        printf("Quantidade inválida de partições.\n");
        return 0;
    }

    /* A fila contém primeiro as folhas e depois os nós internos. */
    TNoV **fila = malloc((size_t)num_p * 2 * sizeof(*fila));
    if (fila == NULL) {
        printf("Não foi possível alocar a árvore de vencedores.\n");
        return 0;
    }
    int inicio = 0, fim = 0, folhas = 0;
    int sucesso = 0;
    FILE *out = NULL;

    for (Lista *p = nome_particoes; folhas < num_p; p = p->prox) {
        if (p == NULL) goto limpar;
        FILE *f = fopen(p->nome, "rb");
        if (f == NULL) {
            printf("Não foi possível abrir a partição %s.\n", p->nome);
            goto limpar;
        }
        TFunc *func = le_funcionario(f);
        TNoV *folha = cria_no_v(func, f);
        if (folha == NULL || ferror(f)) {
            free(folha);
            free(func);
            fclose(f);
            goto limpar;
        }
        fila[fim++] = folha;
        folhas++;
    }

    while (fim - inicio > 1) {
        TNoV *pai = cria_no_v(NULL, NULL);
        if (pai == NULL) goto limpar;
        pai->esq = fila[inicio++];
        pai->dir = fila[inicio++];
        pai->esq->pai = pai;
        pai->dir->pai = pai;
        atualiza_vencedor(pai);
        fila[fim++] = pai;
    }

    out = fopen(nome_arquivo_saida, "wb");
    if (out == NULL) {
        printf("Não foi possível abrir o arquivo de saída %s.\n", nome_arquivo_saida);
        goto limpar;
    }

    TNoV *raiz = fila[inicio];
    for (int pos_saida = 0; raiz->vencedor != NULL; pos_saida++) {
        if (fseek(out, (long)pos_saida * tamanho_registro(), SEEK_SET) != 0) goto limpar;
        salva_funcionario(raiz->vencedor, out);
        if (ferror(out)) goto limpar;

        TNoV *folha = raiz->endVencedor;
        if (fseek(folha->f, (long)folha->aux_p * tamanho_registro(), SEEK_SET) != 0) goto limpar;
        free(folha->vencedor);
        folha->vencedor = le_funcionario(folha->f);
        if (ferror(folha->f)) goto limpar;
        folha->aux_p++;

        for (TNoV *no = folha->pai; no != NULL; no = no->pai) {
            atualiza_vencedor(no);
        }
    }
    sucesso = 1;

limpar:
    for (int i = 0; i < folhas; i++) {
        fclose(fila[i]->f);
        free(fila[i]->vencedor);
    }
    for (int i = 0; i < fim; i++) {
        free(fila[i]);
    }
    free(fila);
    if (out != NULL) {
        if (fclose(out) != 0) sucesso = 0;
        if (!sucesso) remove(nome_arquivo_saida);
    }
    if (sucesso) printf("\nIntercalação com Árvore de Vencedores concluída com sucesso!\n");
    else printf("A intercalação com Árvore de Vencedores não foi concluída.\n");
    return sucesso;
}
