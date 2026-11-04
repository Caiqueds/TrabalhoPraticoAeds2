/*
    Prof. Janniele Aparecida Soares Araújo
    Arcabouço para desenvolvimento do trabalho da disciplina de AEDSII 2020-1, utilizando os exemplos dos livros
    de Schildt, H. C Completo e Total. Ed. McGraw-Hill e de Ferraz, I. N. Programação com Arquivos. Editora Manole Ltda
      sobre o problema de funcionários e algoritmos.
*/

#include <stdlib.h>


#include "ordenacao_busca.h"
#include "lista.h"
#include "particoes.h"
#include "intercalacao.h"
#include "arvore_binaria.h"

static int le_inteiro(int *valor) {
    int resultado = scanf("%d", valor);
    if (resultado == 0) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
        }
    }
    return resultado;
}

static void mostra_menu(void) {
    printf("\n\n**************MENU************** \n\nBASE DE DADOS\n");

    printf("\nDigite 1 para utilizar a base de dados funcionario de teste.");
    printf("\nDigite 2 se desejar imprimir a base de dados funcionario.");
    printf("\nDigite 3 se desejar imprimir o índice primário da base de funcionarios.");


    printf("\n\nINTERPOLAÇÃO\n");
    printf("\nDigite 4 para utilizar a estrutura de interpolação teste.");
    printf("\nDigite 5, 6  para gerar particões ordenadas utilizando, respectivamente, os métodos: classificação interna ou seleção natural.");
    printf("\nDigite 7 para criar uma pilha com o conteúdo das partições geradas.");
    printf("\nDigite 8, 9 para gerar intercalar as partições ordenadas utilizando, respectivamente, os métodos: intercalação basico, intercalacao arvore de vencedores.");

    printf("\n\nINSERTION SORT E ÁRVORE BINÁRIA, ARVORE B\n");
    printf("\nDigite 10 para criar árvores binárias em memória interna e externa.");
    printf("\nDigite 11 para ordenar a base de dados utilizando o método insertion sort em disco. ");
    printf("\nDigite 12 para criar Árvore B+ de prefixo simples. ");

}

static int menu(void) {
    int op;
    printf("\nDigite uma opção (-1 para sair): ");
    int resultado = le_inteiro(&op);
    if (resultado == EOF) return -1;
    if (resultado == 0) return 0;
    return op;

}

int main(){

    /*criando um arquivo lógico para abir para leitura e escrita o arquivo binário*/
    FILE* arq = fopen("base_funcionarios.dat", "wb+");
    if(arq == NULL) {
        printf("Arquivo base_funcionarios.dat não encontrado.\n");
        return 1;
    }

    /*criando um arquivo lógico para abir para leitura e escrita do arquivo de indice primario ordenado*/
    FILE* arqIndicePrimario = fopen("IndicePrimarioFunc.dat", "wb+");
    if(arqIndicePrimario == NULL) {
        printf("Arquivo IndicePrimarioFunc.dat não encontrado.\n");
        return 1;
    }

    /*criando um arquivo lógico para abir para leitura e escrita do arquivo de indice primario não ordenado*/
    FILE* arqIndicePrimarioNaoOrdenado = fopen("IndicePrimarioFuncNaoOrdenado.dat", "wb+");
    if(arqIndicePrimarioNaoOrdenado == NULL) {
        printf("Arquivo IndicePrimarioFuncNaoOrdenado.dat não encontrado.\n");
        return 1;
    }

    /*criando um arquivo lógico para abir para leitura e escrita da árvore binária externa*/
    FILE* arqArvoreBinariaExterna = fopen("ArvoreBinariaExternaFunc.dat", "wb+");
    if(arqArvoreBinariaExterna == NULL) {
        printf("Arquivo ArvoreBinariaExternaFunc.dat não encontrado.\n");
        return 1;
    }


    mostra_menu();
    int op = menu();
    int nFunc = 0;

    int nParticoes = 0, nElementos = 0;
    Lista  * nomes = NULL;

    int crioubase = 0;

    while(op!=-1){
        switch (op){
            case (1):
                //criando a base de dados de funcionários teste e o arquivo de índice primário
                nFunc = 20;
                cria_base_dados_funcionarios_teste(arq, arqIndicePrimario, arqIndicePrimarioNaoOrdenado, nFunc);
                crioubase = 1;
                nParticoes = 0;
                break;
            case (2):
                //imprime a base de dados dos funcionários
                if(crioubase==1)
                    imprime_base_dados_funcionarios(arq, nFunc);
                else  printf("Base de dados inexistente para ser impressa, crie utilizando a opção 1 do menu!!!\n");
                break;
            case (3):
                //imprime o arquivo de índices primários
                if(crioubase==1){
                    printf("\nArquivo ordenado\n ch:rrn\n");
                    imprime_indice_primario_funcionarios(arqIndicePrimario,nFunc);
                    printf("\nArquivo não ordenado\n ch:rrn\n");
                    imprime_indice_primario_funcionarios(arqIndicePrimarioNaoOrdenado,nFunc);
                }else  printf("Indice primario inexistente para ser impresso, será criado após a definição da base de dados utilizando a opção 1 do menu!!!\n");
                break;
            case (4):
                libera(nomes);
                nomes = NULL;
                nParticoes = 0;
                nElementos = 2;
                printf("\nLista inicial com 10 nomes de partições.\n");
                for (int i = 10; i >= 1; i--) {
                    char nome[32];
                    snprintf(nome, sizeof(nome), "p%d.dat", i);
                    nomes = cria(cria_str(nome), nomes);
                }
                imprime(nomes);
                break;
            case (5):
            case (6):
                if (!crioubase) {
                    printf("Crie a base de dados pela opção 1.\n");
                    break;
                }
                if (nomes == NULL) {
                    printf("Prepare a lista de partições pela opção 4.\n");
                    break;
                }

                if (op == 5) {
                    printf("\nGerando partições por classificação interna.\n");
                    nParticoes = classificacao_interna(arq, nomes, nElementos, nFunc);
                } else {
                    int M, tam_reservatorio;
                    printf("\nInforme o tamanho M da memória (registros): ");
                    if (le_inteiro(&M) != 1 || M < 1) {
                        printf("O tamanho da memória deve ser um inteiro positivo.\n");
                        break;
                    }
                    printf("Informe o tamanho do reservatório (registros, no máximo M): ");
                    if (le_inteiro(&tam_reservatorio) != 1 || tam_reservatorio < 1) {
                        printf("O tamanho do reservatório deve ser um inteiro positivo.\n");
                        break;
                    }
                    printf("\nGerando partições por seleção natural.\n");
                    nParticoes = selecao_natural(arq, nomes, M, nFunc, tam_reservatorio);
                }
                break;
            case (7): {
                if (nParticoes <= 0) {
                    printf("Gere as partições pela opção 5 ou 6.\n");
                    break;
                }
                Lista *particao = nomes;
                for (int p = 0; p < nParticoes; p++, particao = particao->prox) {
                    FILE *part = fopen(particao->nome, "rb");
                    if (part == NULL) {
                        printf("Não foi possível abrir a partição %s.\n", particao->nome);
                        break;
                    }
                    int tamParticao = conta_registros_particao(particao->nome);
                    TPilha *pilha = malloc(sizeof(*pilha) * (tamParticao > 0 ? tamParticao : 1));
                    if (pilha == NULL) {
                        fclose(part);
                        printf("Não foi possível alocar a pilha da partição %s.\n", particao->nome);
                        break;
                    }
                    printf("Partição %s\n", particao->nome);
                    imprime_base_dados_funcionarios(part, tamParticao);
                    int topo = -1;
                    cria_pilha_particao(pilha, part, tamParticao, &topo, p);
                    for (int i = 0; i <= topo; i++) {
                        free(pilha[i].info);
                    }
                    free(pilha);
                    fclose(part);
                }
                break;
            }
            case (8):
            case (9): {
                if (nParticoes <= 0) {
                    printf("Gere as partições pela opção 5 ou 6.\n");
                    break;
                }

                char *saida;
                if (op == 8) {
                    saida = "arquivo_intercalado.dat";
                    printf("\nIntercalando as partições pelo método básico.\n");
                    if (!intercalacao_basico(saida, nParticoes, nomes)) break;
                } else {
                    saida = "arquivo_intercalado_arvore.dat";
                    if (!intercalacao_arvore_vencedores(saida, nParticoes, nomes)) break;
                }

                FILE *fi = fopen(saida, "rb");
                if (fi == NULL) {
                    printf("Não foi possível abrir o arquivo %s.\n", saida);
                    break;
                }
                printf("\n--- BASE DE DADOS INTERCALADA ---\n");
                imprime_base_dados_funcionarios(fi, nFunc);
                fclose(fi);
                break;
            }
            case (10):
                //cria árvore binária em memória utilizando o arquivo de índice primário ordenado
                if(crioubase==1){
                    printf("\nArvore binária em memória pelo arquivo de  índice primário ordenado\n <nopai <noesq><nodireita> >\n");
                    cria_arvore_binaria_indice_primario(arqIndicePrimario,nFunc);
                    printf("\nArvore binária em memória pelo arquivo de índice primário não ordenado\n <nopai <noesq><nodireita> >\n");
                    cria_arvore_binaria_indice_primario(arqIndicePrimarioNaoOrdenado,nFunc);
                    printf("\nArvore binária externa pelo arquivo de base de dados do funcionário\n <nopai <noesq><nodireita> >\n");

                    arqArvoreBinariaExterna = freopen(
                        "ArvoreBinariaExternaFunc.dat",
                            "wb+",
                        arqArvoreBinariaExterna
                    );

                    if (arqArvoreBinariaExterna == NULL) {
                        printf("Erro ao recriar o arquivo da arvore externa.\n");
                        break;
                    }

                    criar_arvore_binaria_externa(arq,arqArvoreBinariaExterna);
                }else  printf("Arquivo de índice primario inexistente para a criação da arvore binaria, crie utilizando a opção 1 do menu!!!\n");
                break;
            case (11):
                 //ordenando a base de dados utilizando ordenação direta no arquivo com o método inseriton sort
                if(crioubase==1){
                    insertion_sort_disco(arq, nFunc);
                    imprime_base_dados_funcionarios(arq, nFunc);
                    //procurando por um funcionário na base de dados usando o método de busca binária
                    int busca_codigo;
                    printf("\n\nInforme o codigo do funcionário procurado (digite -1 caso não queira procurar um funcionário): ");
                    if (le_inteiro(&busca_codigo) != 1) break;
                    if(busca_codigo!= -1){
                        TFunc* func = busca_binaria(busca_codigo, arq, nFunc);

                        if (func != NULL) {
                            imprime_funcionario(func);
                            free(func);
                        } else {
                            printf("Funcionario com codigo %d nao encontrado.\n",busca_codigo);
                        }

                    }
                }else printf("Base de dados inexistente para ser ordenada, crie utilizando a opção 1 do menu!!!\n");
                break;
            case (12):
                printf("Método a ser implementado na parte II do trabalho.\n");
                if(crioubase==1){
                    ;//criar_arvore_B+();
                }else printf("Base de dados inexistente para ser ordenada, crie utilizando a opção 1 do menu!!!\n");
                break;
            default:
                printf("\nOpção inválida!");
        }
        op = menu();
    }

    libera(nomes);

    fclose(arqIndicePrimario);
    fclose(arqIndicePrimarioNaoOrdenado);
    fclose(arqArvoreBinariaExterna);
    fclose(arq);

    return 0;
}