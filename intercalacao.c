#include "intercalacao.h"
#include "funcionarios.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

void intercalacao_basico(char *nome_arquivo_saida, int num_p, Lista *nome_particoes) {

    int fim = 0; //variavel que controla fim do procedimento
    FILE *out; //declara ponteiro para arquivo

    //abre arquivo de saida para escrita
    if ((out = fopen(nome_arquivo_saida, "wb")) == NULL) {
        printf("Erro ao abrir arquivo de sa?da\n");
    } else {
        //cria vetor de particoes
        TVet v[num_p];

        //abre arquivos das particoes, colocando variavel de arquivo no campo f do vetor
        //e primeiro funcionario do arquivo no campo func do vetor
        for (int i=0; i < num_p; i++) {
            v[i].f = fopen(nome_particoes->nome, "rb");
            v[i].aux_p = 0;
            if (v[i].f != NULL) {
                fseek(v[i].f, v[i].aux_p * tamanho_registro(), SEEK_SET);
                TFunc *f = le_funcionario(v[i].f);
                if (f == NULL) {
                    //arquivo estava vazio
                    //coloca HIGH VALUE nessa posi??o do vetor
                    v[i].func = funcionario(INT_MAX, "","","",0);
                }
                else {
                    //conseguiu ler funcionario, coloca na posi??o atual do vetor
                    v[i].func = f;
                }
            }
            else {
                fim = 1;
            }
            nome_particoes = nome_particoes->prox;
        }

        int aux = 0;
        while (!(fim)) { //conseguiu abrir todos os arquivos
            int menor = INT_MAX;
            int pos_menor;
            //encontra o funcionario com menor chave no vetor
            for(int i = 0; i < num_p; i++){
                if(v[i].func->cod < menor){
                    menor = v[i].func->cod;
                    pos_menor = i;
                }
            }
            if (menor == INT_MAX) {
                fim = 1; //terminou processamento
            }
            else {
                //salva funcionario no arquivo de saída
                fseek(out, aux * tamanho_registro(), SEEK_SET);
                salva_funcionario(v[pos_menor].func, out);
                //atualiza posição pos_menor do vetor com pr?ximo funcionario do arquivo
                v[pos_menor].aux_p++;
                fseek(v[pos_menor].f, v[pos_menor].aux_p * tamanho_registro(), SEEK_SET);
                TFunc *f = le_funcionario(v[pos_menor].f);
                aux++;
                if (f == NULL) {
                    //arquivo estava vazio
                    //coloca HIGH VALUE nessa posiçao do vetor
                    v[pos_menor].func = funcionario(INT_MAX, "", "", "",0.0);
                }
                else {
                    v[pos_menor].func = f;
                }

            }
        }

        //fecha arquivos das partiÇões de entrada
        for(int i = 0; i < num_p; i++){
            fclose(v[i].f);
        //    free(v[i].func);
        }
        //fecha arquivo de saída
        fclose(out);
    }


}

// Função auxiliar para criar os nós folha da árvore
TNoV* cria_no_v(TFunc *func, FILE *f) {
    TNoV *novo = (TNoV*) malloc(sizeof(TNoV));
    novo->vencedor = func;
    novo->endVencedor = novo; // A folha aponta para si mesma como origem
    novo->f = f;
    novo->aux_p = 1; // o registro 0 ja foi lido na inicializacao
    novo->pai = NULL;
    novo->esq = NULL;
    novo->dir = NULL;
    return novo;
}



void intercalacao_arvore_vencedores(char *nome_arquivo_saida, int num_p, Lista *nome_particoes) {
    FILE *out = fopen(nome_arquivo_saida, "wb");
    if (!out) {
        printf("Erro ao abrir arquivo de saída\n");
        return;
    }

    // Fila para auxiliar na construção da árvore (tamanho máximo 2*N-1)
    TNoV *fila[num_p * 2];
    int inicio = 0, fim = 0;
    Lista *p = nome_particoes;

    // PASSO 1: Inicialização (Criar as folhas com o primeiro registo de cada partição)
    for (int i = 0; i < num_p; i++) {
        FILE *f = fopen(p->nome, "rb");
        TFunc *func = NULL;
        if (f != NULL) {
            func = le_funcionario(f);
        }
        fila[fim++] = cria_no_v(func, f);
        p = p->prox;
    }

    // PASSO 2: Combinação (Juntar pares de nós até restar apenas 1 nó raiz na fila)
    while (fim - inicio > 1) {
        TNoV *esq = fila[inicio++];
        TNoV *dir = fila[inicio++];

        TNoV *pai = (TNoV*) malloc(sizeof(TNoV));
        pai->esq = esq;
        pai->dir = dir;
        pai->f = NULL;
        pai->aux_p = 0;
        pai->pai = NULL;
        esq->pai = pai;
        dir->pai = pai;

        int cod_esq = (esq->vencedor != NULL) ? esq->vencedor->cod : INT_MAX;
        int cod_dir = (dir->vencedor != NULL) ? dir->vencedor->cod : INT_MAX;

        if (cod_esq <= cod_dir) {
            pai->vencedor = esq->vencedor;
            pai->endVencedor = esq->endVencedor;
        } else {
            pai->vencedor = dir->vencedor;
            pai->endVencedor = dir->endVencedor;
        }

        fila[fim++] = pai;
    }

    // PASSO 3: Intercalação (Gravação do menor registo e recalculamento dos vencedores)
    TNoV *raiz = fila[inicio];
    int pos_saida = 0; // RRN do proximo registro a gravar na saida

    while (raiz != NULL && raiz->vencedor != NULL && raiz->vencedor->cod != INT_MAX) {
        // 1. Grava o menor elemento no ficheiro final
        fseek(out, pos_saida * tamanho_registro(), SEEK_SET);
        salva_funcionario(raiz->vencedor, out);
        pos_saida++;

        // 2. Encontra a folha de onde veio esse vencedor
        TNoV *folha = raiz->endVencedor;

        // 3. Liberta o registo que acabou de ser gravado
        free(folha->vencedor);

        // 4. Lê o próximo funcionário da mesma partição
        // posiciona no proximo registro (registros ocupam tamanho_registro() bytes, com padding)
        fseek(folha->f, folha->aux_p * tamanho_registro(), SEEK_SET);
        folha->vencedor = le_funcionario(folha->f);
        folha->aux_p++;

        // Se a partição chegou ao fim, atribui um nó Dummy (com código infinito)
        if (folha->vencedor == NULL) {
            folha->vencedor = (TFunc*) calloc(1, sizeof(TFunc));
            folha->vencedor->cod = INT_MAX;
        }

        // 5. Sobe a árvore recalculando os vencedores até à raiz
        TNoV *atual = folha->pai;
        while (atual != NULL) {
            int cod_esq = (atual->esq != NULL && atual->esq->vencedor != NULL) ? atual->esq->vencedor->cod : INT_MAX;
            int cod_dir = (atual->dir != NULL && atual->dir->vencedor != NULL) ? atual->dir->vencedor->cod : INT_MAX;

            if (cod_esq <= cod_dir) {
                atual->vencedor = atual->esq->vencedor;
                atual->endVencedor = atual->esq->endVencedor;
            } else {
                atual->vencedor = atual->dir->vencedor;
                atual->endVencedor = atual->dir->endVencedor;
            }

            atual = atual->pai;
        }
    }

    // PASSO 4: Limpeza de Ficheiros e Memória
    for (int i = 0; i < num_p; i++) {
        if (fila[i]->f != NULL) {
            fclose(fila[i]->f);
            fila[i]->f = NULL;
        }
        if (fila[i]->vencedor != NULL) {
            free(fila[i]->vencedor);
            fila[i]->vencedor = NULL;
        }
    }

    for (int i = 0; i < fim; i++) {
        if (fila[i] != NULL) {
            free(fila[i]);
        }
    }

    fclose(out);

    printf("\nIntercalacao com Arvore de Vencedores concluida com sucesso!\n");
}


