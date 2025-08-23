/*
Programa concorrente para calcular o produto interno de dois vetores de floats
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

//variáveis globais
float *vetA, *vetB; //vetores de entrada
long int n; //tamanho dos vetores
int nthreads; //número de threads

//estrutura de argumentos para a thread
typedef struct {
   int id; //identificador da thread
} t_Args;

//função executada pelas threads
void *ProdutoInterno(void *arg) {
   t_Args *args = (t_Args*) arg;
   long int ini, fim, bloco;
   double soma_local=0, *ret;

   bloco = n / nthreads;
   ini = args->id * bloco;
   fim = (args->id == nthreads-1) ? n : ini + bloco;

   for(long int i=ini; i<fim; i++) {
      soma_local += (double)vetA[i] * (double)vetB[i];
   }

   ret = (double*) malloc(sizeof(double));
   if(ret==NULL) {
      fprintf(stderr, "--ERRO: malloc thread\n");
      pthread_exit(NULL);
   }
   *ret = soma_local;

   free(arg);
   pthread_exit((void*) ret);
}

//função principal
int main(int argc, char *argv[]) {
   pthread_t *tid_sistema;
   double *retorno, soma_conc=0, soma_seq, inicio, fim_tempo, delta;
   FILE *arq;
   size_t ret;

   if(argc < 3) {
      fprintf(stderr, "Digite: %s <arquivo de entrada> <numero de threads>\n", argv[0]);
      return 1;
   }

   //abre o arquivo binário
   arq = fopen(argv[1], "rb");
   if(!arq) {
      fprintf(stderr, "Erro de abertura do arquivo %s\n", argv[1]);
      return 2;
   }

   //lê dimensão
   ret = fread(&n, sizeof(long int), 1, arq);
   if(!ret) { fprintf(stderr, "Erro ao ler dimensão\n"); return 3; }

   //aloca e lê vetores
   vetA = (float*) malloc(sizeof(float) * n);
   vetB = (float*) malloc(sizeof(float) * n);
   if(!vetA || !vetB) {
      fprintf(stderr, "Erro de malloc dos vetores\n");
      return 4;
   }
   ret = fread(vetA, sizeof(float), n, arq);
   ret = fread(vetB, sizeof(float), n, arq);
   if(ret < n) { fprintf(stderr, "Erro ao ler vetores\n"); return 5; }

   //lê produto interno sequencial
   ret = fread(&soma_seq, sizeof(double), 1, arq);
   fclose(arq);

   //lê número de threads
   nthreads = atoi(argv[2]);
   if(nthreads > n) nthreads = n;

   tid_sistema = (pthread_t*) malloc(sizeof(pthread_t) * nthreads);
   if(!tid_sistema) { fprintf(stderr, "Erro malloc tid\n"); return 6; }

   //dispara as threads
   GET_TIME(inicio);
   for(int i=0; i<nthreads; i++) {
      t_Args *args = (t_Args*) malloc(sizeof(t_Args));
      if(args==NULL) { fprintf(stderr, "Erro malloc args\n"); return 7; }
      args->id = i;
      if(pthread_create(&tid_sistema[i], NULL, ProdutoInterno, (void*) args)) {
         fprintf(stderr, "Erro pthread_create\n"); return 8;
      }
   }

   //espera as threads
   for(int i=0; i<nthreads; i++) {
      if(pthread_join(tid_sistema[i], (void**) &retorno)) {
         fprintf(stderr, "Erro pthread_join\n"); return 9;
      }
      soma_conc += *retorno;
      free(retorno);
   }
   GET_TIME(fim_tempo);

   delta = fim_tempo - inicio;

   //imprime resultados
   printf("Produto interno concorrente = %.15lf\n", soma_conc);
   printf("Produto interno sequencial  = %.15lf\n", soma_seq);

   double erro_rel = (soma_seq - soma_conc) / soma_seq;
   printf("Variacao relativa           = %.10e\n", erro_rel);
   printf("Tempo de execucao           = %e segundos\n", delta);

   //libera memória
   free(vetA);
   free(vetB);
   free(tid_sistema);

   return 0;
}