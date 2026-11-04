
#include "funcionarios.h"
#include <stdlib.h>

#include "particoes.h"

int classificacao_interna(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc) {
    if (M <= 0 || nFunc <= 0) return 0;
    TFunc **v = malloc(sizeof(*v) * M);
    if (v == NULL) {
        printf("Não foi possível alocar memória para a classificação.\n");
        return 0;
    }

    int reg = 0, num_particoes = 0;
    while (reg < nFunc && nome_arquivos_saida != NULL) {
        int qtd = 0;
        while (qtd < M && reg < nFunc) {
            if (fseek(arq, (long)reg * tamanho_registro(), SEEK_SET) != 0) break;
            TFunc *func = le_funcionario(arq);
            if (func == NULL) break;
            v[qtd++] = func;
            reg++;
        }

        int sucesso = qtd == M || reg == nFunc;
        if (sucesso) {
            for (int j = 1; j < qtd; j++) {
                TFunc *f = v[j];
                int i = j - 1;
                while (i >= 0 && v[i]->cod > f->cod) {
                    v[i + 1] = v[i];
                    i--;
                }
                v[i + 1] = f;
            }

            printf("\n%s\n", nome_arquivos_saida->nome);
            FILE *p = fopen(nome_arquivos_saida->nome, "wb");
            sucesso = p != NULL;
            if (p != NULL) {
                for (int i = 0; i < qtd; i++) {
                    if (fseek(p, (long)i * tamanho_registro(), SEEK_SET) != 0) {
                        sucesso = 0;
                        break;
                    }
                    salva_funcionario(v[i], p);
                    if (ferror(p)) {
                        sucesso = 0;
                        break;
                    }
                    imprime_funcionario(v[i]);
                }
                if (fclose(p) != 0) sucesso = 0;
            }
        }
        for (int i = 0; i < qtd; i++) free(v[i]);
        if (!sucesso) {
            free(v);
            printf("A classificação interna não foi concluída.\n");
            return 0;
        }
        num_particoes++;
        nome_arquivos_saida = nome_arquivos_saida->prox;
    }

    free(v);
    if (reg != nFunc) {
        printf("Faltaram nomes de arquivos para gerar as partições.\n");
        return 0;
    }
    return num_particoes;
}

/*Começo seleçao natural*/

/* Retorna a posicao do registro de menor chave no array em memoria
   (posicoes vazias sao NULL). Retorna -1 se a memoria estiver vazia. */
int posicao_menor_chave(TFunc **memoria, int M) {
    int pos = -1;
    for (int i = 0; i < M; i++) {
        if (memoria[i] != NULL && (pos == -1 || memoria[i]->cod < memoria[pos]->cod))
            pos = i;
    }
    return pos;
}

int selecao_natural(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc, int n) {
    if (arq == NULL || nome_arquivos_saida == NULL || M < 1 || n < 1 || nFunc < 1) return 0;
    if (n > M) {
        /* o conteudo do reservatorio precisa caber no array em memoria (passo 7) */
        printf("Reservatorio (%d) maior que a memoria (%d): usando reservatorio = %d.\n", n, M, M);
        n = M;
    }

    rewind(arq);

    /* reservatorio fica em memoria secundaria (arquivo em disco) */
    FILE *reservatorio = fopen("reservatorio.dat", "wb+");
    if (reservatorio == NULL) {
        printf("Erro ao criar o arquivo do reservatorio\n");
        return 0;
    }

    TFunc **memoria = calloc(M, sizeof(*memoria));
    if (memoria == NULL) {
        fclose(reservatorio);
        remove("reservatorio.dat");
        printf("Não foi possível alocar memória para a seleção natural.\n");
        return 0;
    }

    FILE *p = NULL;
    int proximo = 0;          /* indice do proximo registro a ser lido da entrada */
    int no_reservatorio = 0;  /* quantidade de registros no reservatorio */
    int num_particoes = 0;
    Lista *atual = nome_arquivos_saida;
    Lista *ultimo = NULL;

    /* Passo 1: ler M registros do arquivo para a memoria */
    for (int i = 0; i < M && proximo < nFunc; i++) {
        if (fseek(arq, (long)proximo * tamanho_registro(), SEEK_SET) != 0) goto falha;
        memoria[i] = le_funcionario(arq);
        if (memoria[i] == NULL) goto falha;
        proximo++;
    }

    /* cada volta deste laco gera uma particao */
    while (posicao_menor_chave(memoria, M) != -1) {
        /* se acabaram os nomes da lista de particoes, cria um novo e anexa no final */
        if (atual == NULL) {
            char nome[20];
            sprintf(nome, "p%d.dat", num_particoes + 1);
            atual = cria(cria_str(nome), NULL);
            ultimo->prox = atual;
        }

        p = fopen(atual->nome, "wb+");
        if (p == NULL) {
            printf("Erro ao criar a particao %s\n", atual->nome);
            goto falha;
        }
        printf("\n%s\n", atual->nome);

        int gravados = 0;
        int pos;
        /* Passo 2: selecionar o registro r de menor chave da memoria */
        while ((pos = posicao_menor_chave(memoria, M)) != -1) {
            TFunc *r = memoria[pos];

            /* Passo 3: gravar r na particao de saida */
            if (fseek(p, (long)gravados * tamanho_registro(), SEEK_SET) != 0) goto falha;
            salva_funcionario(r, p);
            if (ferror(p)) goto falha;
            imprime_funcionario(r);
            gravados++;
            int ultima_chave = r->cod;
            free(r);
            memoria[pos] = NULL;

            /* Passos 4 e 5: substituir r pelo proximo registro da entrada.
               Se a chave dele for menor que a recem-gravada, vai para o reservatorio
               e le o proximo. Com o reservatorio cheio, nao se le mais nada:
               a memoria e esvaziada (em ordem) na particao atual. */
            while (no_reservatorio < n && proximo < nFunc) {
                if (fseek(arq, (long)proximo * tamanho_registro(), SEEK_SET) != 0) goto falha;
                TFunc *novo = le_funcionario(arq);
                if (novo == NULL) goto falha;
                proximo++;
                if (novo->cod >= ultima_chave) {
                    memoria[pos] = novo;
                    break;
                }
                if (fseek(reservatorio, (long)no_reservatorio * tamanho_registro(), SEEK_SET) != 0) {
                    free(novo);
                    goto falha;
                }
                salva_funcionario(novo, reservatorio);
                free(novo);
                if (ferror(reservatorio)) goto falha;
                no_reservatorio++;
            }
        }

        /* Passo 7: fechar a particao de saida */
        int resultado = fclose(p);
        p = NULL;
        if (resultado != 0) goto falha;
        num_particoes++;
        ultimo = atual;
        atual = atual->prox;

        /* copiar os registros do reservatorio para o array em memoria */
        for (int i = 0; i < no_reservatorio; i++) {
            if (fseek(reservatorio, (long)i * tamanho_registro(), SEEK_SET) != 0) goto falha;
            memoria[i] = le_funcionario(reservatorio);
            if (memoria[i] == NULL) goto falha;
        }
        /* esvaziar o reservatorio: o proximo lote sobrescreve a partir do inicio */
        no_reservatorio = 0;

        /* completar a memoria com registros da entrada, se ainda houver */
        for (int i = 0; i < M && proximo < nFunc; i++) {
            if (memoria[i] == NULL) {
                if (fseek(arq, (long)proximo * tamanho_registro(), SEEK_SET) != 0) goto falha;
                memoria[i] = le_funcionario(arq);
                if (memoria[i] == NULL) goto falha;
                proximo++;
            }
        }
    }

    free(memoria);
    fclose(reservatorio);
    remove("reservatorio.dat");

    printf("\nTotal de particoes geradas: %d\n", num_particoes);
    return num_particoes;

falha:
    if (p != NULL) fclose(p);
    for (int i = 0; i < M; i++) free(memoria[i]);
    free(memoria);
    fclose(reservatorio);
    remove("reservatorio.dat");
    printf("A seleção natural não foi concluída.\n");
    return 0;
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
