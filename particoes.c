
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

void selecao_natural(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc, int n){
    /*Utliza RESERVATORIO 
    RESERVATORIO é em memoria secundaria.
    Condiçao para parada: .Reservatório Cheio:Não há mais espaço livre na memória secundária reservada.
                          .Fim dos Registros: Terminarem todos os registros do arquivo de entrada
                     
    O LOOP:
    1 - Ler M registros do arquivo para a memória
    2 - Selecionar no array em memória o registro 'r' com menor chave
    3 - Gravar o registro 'r' na partição de saída
    4 - Substituir no array o registro 'r' pelo próximo do arquivo de entrada
    5 - Se a chave deste for menor que a recém-gravada, coloca ela no reservatório e substituir 'r' pelo próximo registro
    6 - Se ainda tiver espaço livro no reservatório, volta no passo 2
    Caso nao tiver espaço no reservatório:
    -Fechar a partição de saída
    -Copiar os registros do reservatório para o array em memória
    -Esvaziar o reservatório
    -Abrir nova partição de saída
    -Voltar no Passo 2
    */
}
