/*
Programa auxiliar para gerar dois vetores de floats e calcular seu produto interno
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 1000 //valor máximo de um elemento do vetor
//#define LOG //descomente para imprimir os vetores na tela

int main(int argc, char*argv[]) {
   float *vetA, *vetB; //vetores que serão gerados
   long int n; //qtde de elementos dos vetores
   float elemA, elemB; //valores gerados
   double prod=0; //produto interno
   FILE * descritorArquivo; //descritor do arquivo de saída
   size_t ret; //retorno da funcao de escrita

   //recebe os argumentos de entrada
   if(argc < 3) {
      fprintf(stderr, "Digite: %s <dimensao> <nome arquivo saida>\n", argv[0]);
      return 1;
   }
   n = atol(argv[1]);

   //aloca memoria para os vetores
   vetA = (float*) malloc(sizeof(float) * n);
   vetB = (float*) malloc(sizeof(float) * n);
   if(!vetA || !vetB) {
      fprintf(stderr, "Erro de alocao da memoria dos vetores\n");
      return 2;
   }

   //preenche os vetores com valores float aleatórios
   srand(time(NULL));
   for(long int i=0; i<n; i++) {
        elemA = (rand() % MAX) / 3.0;
        elemB = (rand() % MAX) / 3.0;
        vetA[i] = elemA;
        vetB[i] = elemB;
        prod += (double)elemA * (double)elemB;
   }

   //imprimir na saída padrão os vetores gerados (opcional)
   #ifdef LOG
   fprintf(stdout, "%ld\n", n);
   for(long int i=0; i<n; i++) {
      fprintf(stdout, "%f ",vetA[i]);
   }
   fprintf(stdout, "\n");
   for(long int i=0; i<n; i++) {
      fprintf(stdout, "%f ",vetB[i]);
   }
   fprintf(stdout, "\n");
   fprintf(stdout, "%lf\n", prod);
   #endif

   //escreve os dados no arquivo binário
   descritorArquivo = fopen(argv[2], "wb");
   if(!descritorArquivo) {
      fprintf(stderr, "Erro de abertura do arquivo\n");
      return 3;
   }
   //escreve a dimensão
   ret = fwrite(&n, sizeof(long int), 1, descritorArquivo);
   //escreve os vetores
   ret = fwrite(vetA, sizeof(float), n, descritorArquivo);
   ret = fwrite(vetB, sizeof(float), n, descritorArquivo);
   //escreve o produto interno
   ret = fwrite(&prod, sizeof(double), 1, descritorArquivo);

   fclose(descritorArquivo);
   free(vetA);
   free(vetB);
   return 0;
} 
