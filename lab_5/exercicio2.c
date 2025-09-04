/*
Disciplina: Programacao Concorrente
Prof.: Silvana Rossetto
Codigo: Comunicação entre threads com variável compartilhada,
        exclusao mutua (mutex) e sincronização condicional (cond var) 

Nome: Richard Nozari Cassol Lahm Piccoli
DRE: 119194605
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

long int soma = 0;            // variável compartilhada
pthread_mutex_t mutex; 
pthread_cond_t cond; 
int precisa_imprimir = 0;      // flag para controle
long int limite = 0;           // valor máximo esperado da soma
int terminou = 0;              // flag de término
long int ultimo_impresso = 0; // guarda último múltiplo impresso

// funcao executada pelas threads de soma
void *ExecutaTarefa(void *arg) {
    long int id = (long int)arg;
    printf("Thread %ld esta executando...\n", id);

    for (int i = 0; i < 100000; i++) {
        pthread_mutex_lock(&mutex);

        soma++;  

        if (soma % 1000 == 0) {
            precisa_imprimir = 1;
            pthread_cond_signal(&cond); // acorda thread extra
            while (precisa_imprimir && !terminou) {
                pthread_cond_wait(&cond, &mutex);
            }
        }

        pthread_mutex_unlock(&mutex);
    }

    printf("Thread %ld terminou!\n", id);
    pthread_exit(NULL);
}

// funcao executada pela thread extra
void *extra(void *arg) {
    printf("Extra esta executando...\n");

    while (1) {
        pthread_mutex_lock(&mutex);

        while (!precisa_imprimir && soma < limite) {
            pthread_cond_wait(&cond, &mutex);
        }

        if (soma >= limite) {
            terminou = 1;
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&mutex);
            break;
        }

        if (precisa_imprimir) {
            // imprime todos os múltiplos de 1000 que ficaram para trás
            while (ultimo_impresso + 1000 <= soma) {
                ultimo_impresso += 1000;
                printf("soma = %ld\n", ultimo_impresso);
            }
            precisa_imprimir = 0;
            pthread_cond_broadcast(&cond);
        }

        pthread_mutex_unlock(&mutex);
    }

    printf("Extra terminou!\n");
    pthread_exit(NULL);
}

// fluxo principal
int main(int argc, char *argv[]) {
    pthread_t *tid;
    int nthreads;

    if (argc < 2) {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);

    limite = 100000 * nthreads;

    tid = (pthread_t *)malloc(sizeof(pthread_t) * (nthreads + 1));
    if (tid == NULL) {puts("ERRO--malloc"); return 2;}

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    for (long int t = 0; t < nthreads; t++) {
        if (pthread_create(&tid[t], NULL, ExecutaTarefa, (void *)t)) {
            printf("--ERRO: pthread_create()\n"); exit(-1);
        }
    }

    if (pthread_create(&tid[nthreads], NULL, extra, NULL)) {
        printf("--ERRO: pthread_create()\n"); exit(-1);
    }

    for (int t = 0; t < nthreads + 1; t++) {
        if (pthread_join(tid[t], NULL)) {
            printf("--ERRO: pthread_join()\n"); exit(-1);
        }
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    printf("Valor final de soma = %ld\n", soma);
    return 0;
}
