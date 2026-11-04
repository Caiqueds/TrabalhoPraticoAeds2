
#include "funcionarios.h"
#include <stdlib.h>
#include <limits.h>

#include "particoes.h"

void classificacao_interna(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc) {
    rewind(arq); //posiciona cursor no inicio do arquivo


    int reg = 0;

    while (reg != nFunc) {
        //le o arquivo e coloca no vetor
        TFunc *v[M];
        int i = 0;
        while (!feof(arq)) {
            fseek(arq, (reg) * tamanho_registro(), SEEK_SET);
            v[i] = le_funcionario(arq);
       //     imprime_funcionario(v[i]);
            i++;
            reg++;
            if(i>=M) break;
        }

        //ajusta tamanho M caso arquivo de entrada tenha terminado antes do vetor
        if (i != M) {
            M = i;
        }

        //faz ordenacao
        for (int j = 1; j < M; j++) {
            TFunc *f = v[j];
            i = j - 1;
            while ((i >= 0) && (v[i]->cod > f->cod)) {
                v[i + 1] = v[i];
                i = i - 1;
            }
            v[i + 1] = f;
        }

        //cria arquivo de particao e faz gravacao
        char *nome_particao = nome_arquivos_saida->nome;
        nome_arquivos_saida = nome_arquivos_saida->prox;
        printf("\n%s\n", nome_particao);
        FILE *p;
        if ((p = fopen(nome_particao, "wb+")) == NULL) {
            printf("Erro criar arquivo de saida\n");
        } else {
            for (int i = 0; i < M; i++) {
                fseek(p, (i) * tamanho_registro(), SEEK_SET);
                salva_funcionario(v[i], p);
                imprime_funcionario(v[i]);
            }
            fclose(p);
        }
        for(int jj = 0; jj<M; jj++)
            free(v[jj]);
    }
}

/* ===================== SELECAO NATURAL ===================== */

#define NOME_RESERVATORIO "reservatorio.dat"

/* Le o registro de indice 'pos' do arquivo de entrada (NULL se nao existir). */
static TFunc *le_registro_entrada(FILE *arq, int pos) {
    fseek(arq, pos * tamanho_registro(), SEEK_SET);
    return le_funcionario(arq);
}

/* Retorna a posicao do registro de menor chave no array em memoria
   (posicoes vazias sao NULL). Retorna -1 se a memoria estiver vazia. */
static int posicao_menor_chave(TFunc **memoria, int M) {
    int pos = -1;
    for (int i = 0; i < M; i++) {
        if (memoria[i] != NULL && (pos == -1 || memoria[i]->cod < memoria[pos]->cod))
            pos = i;
    }
    return pos;
}

/* Devolve o nome da proxima particao. Se a lista de nomes acabar,
   cria um novo nome "pX.dat" e o anexa ao final da lista. */
static char *proximo_nome_particao(Lista **atual, Lista **ultimo, int num_particao) {
    if (*atual == NULL) {
        char buffer[32];
        sprintf(buffer, "p%d.dat", num_particao);
        Lista *novo = cria(cria_str(buffer), NULL);
        (*ultimo)->prox = novo;
        *atual = novo;
    }
    char *nome = (*atual)->nome;
    *ultimo = *atual;
    *atual = (*atual)->prox;
    return nome;
}

int selecao_natural(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc, int n) {
    if (M < 1) M = 1;
    if (n < 1) n = 1;
    if (n > M) {
        /* o conteudo do reservatorio precisa caber no array em memoria (passo 7) */
        printf("Reservatorio (%d) maior que a memoria (%d): usando reservatorio = %d.\n", n, M, M);
        n = M;
    }

    rewind(arq);

    /* reservatorio fica em memoria secundaria (arquivo em disco) */
    FILE *reservatorio = fopen(NOME_RESERVATORIO, "wb+");
    if (reservatorio == NULL) {
        printf("Erro ao criar o arquivo do reservatorio\n");
        return 0;
    }

    TFunc **memoria = (TFunc **) calloc(M, sizeof(TFunc *));
    int proximo = 0;          /* indice do proximo registro a ser lido da entrada */
    int no_reservatorio = 0;  /* quantidade de registros no reservatorio */
    int num_particoes = 0;

    /* Passo 1: ler M registros do arquivo para a memoria */
    for (int i = 0; i < M && proximo < nFunc; i++)
        memoria[i] = le_registro_entrada(arq, proximo++);

    Lista *nome_atual = nome_arquivos_saida;
    Lista *ultimo_nome = nome_arquivos_saida;

    /* cada volta deste laco gera uma particao */
    while (posicao_menor_chave(memoria, M) != -1) {
        char *nome_particao = proximo_nome_particao(&nome_atual, &ultimo_nome, num_particoes + 1);
        FILE *p = fopen(nome_particao, "wb+");
        if (p == NULL) {
            printf("Erro ao criar a particao %s\n", nome_particao);
            break;
        }
        printf("\n%s\n", nome_particao);

        int gravados = 0;
        int pos;
        /* Passo 2: selecionar o registro r de menor chave da memoria */
        while ((pos = posicao_menor_chave(memoria, M)) != -1) {
            TFunc *r = memoria[pos];

            /* Passo 3: gravar r na particao de saida */
            fseek(p, gravados * tamanho_registro(), SEEK_SET);
            salva_funcionario(r, p);
            imprime_funcionario(r);
            gravados++;
            int ultima_chave = r->cod;
            free(r);
            memoria[pos] = NULL;

            /* Passos 4 e 5: substituir r pelo proximo registro da entrada.
               Se a chave dele for menor que a recem-gravada, vai para o reservatorio
               e le-se o proximo. Com o reservatorio cheio, nao se le mais nada:
               a memoria e esvaziada (em ordem) na particao atual. */
            while (no_reservatorio < n && proximo < nFunc) {
                TFunc *novo = le_registro_entrada(arq, proximo++);
                if (novo == NULL) break;
                if (novo->cod >= ultima_chave) {
                    memoria[pos] = novo;
                    break;
                }
                fseek(reservatorio, no_reservatorio * tamanho_registro(), SEEK_SET);
                salva_funcionario(novo, reservatorio);
                no_reservatorio++;
                free(novo);
            }
        }

        /* Passo 7: fechar a particao de saida */
        fclose(p);
        num_particoes++;

        /* copiar os registros do reservatorio para o array em memoria */
        for (int i = 0; i < no_reservatorio; i++) {
            fseek(reservatorio, i * tamanho_registro(), SEEK_SET);
            memoria[i] = le_funcionario(reservatorio);
        }
        /* esvaziar o reservatorio */
        no_reservatorio = 0;
        reservatorio = freopen(NOME_RESERVATORIO, "wb+", reservatorio);

        /* completar a memoria com registros da entrada, se ainda houver */
        for (int i = 0; i < M && proximo < nFunc; i++)
            if (memoria[i] == NULL)
                memoria[i] = le_registro_entrada(arq, proximo++);
    }

    free(memoria);
    if (reservatorio) fclose(reservatorio);
    remove(NOME_RESERVATORIO);

    printf("\nTotal de particoes geradas: %d\n", num_particoes);
    return num_particoes;
}

int conta_registros_particao(char *nome_particao) {
    FILE *p = fopen(nome_particao, "rb");
    if (p == NULL) return 0;
    int qtd = 0;
    TFunc *f;
    while (1) {
        fseek(p, qtd * tamanho_registro(), SEEK_SET);
        f = le_funcionario(p);
        if (f == NULL) break;
        free(f);
        qtd++;
    }
    fclose(p);
    return qtd;
}
