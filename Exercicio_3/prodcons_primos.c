#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <unistd.h>

typedef long long ll;

// ---------- primalidade (fornecida no enunciado) ---------- 
int ehPrimo(long long int n) {
    long long i;
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    long long lim = (long long) sqrt((double)n) + 1;
    for (i = 3; i <= lim; i += 2)
        if (n % i == 0) return 0;
    return 1;
}

// ---------- buffer e semáforos ---------- 
int M;             // tamanho do buffer
ll *buffer;        // buffer array
int buf_count = 0; // número de itens atualmente no buffer
int buf_out = 0;   // índice de remoção (circular)
int buf_in = 0;    // índice de inserção (não muito usado porque produtor insere em lote)

sem_t sem_items;        // conta itens disponíveis (consumidores aguardam aqui)
sem_t sem_mutex;        // exclusão mútua para acessar buffer_count, indices, etc.
sem_t sem_buffer_empty; // sinaliza que buffer está vazio (produtora espera por isso)
sem_t sem_total;        // proteção para atualizar total_primos

// controle da execução
int N;                  // total a produzir
int C;                  // número de consumidores

// estatísticas 
int *primos_por_consumidor; // vetor de tamanho C
int total_primos = 0;

// produtor: insere lotes (espera buffer vazio antes de inserir)
void *produtor(void *arg) {
    int produced = 0;
    ll next = 1;

    // produzir números 1..N em lotes de até M
    while (produced < N) {
        // aguardar buffer vazio
        sem_wait(&sem_buffer_empty);

        // calcular quantidade a inserir neste lote
        int take = (N - produced >= M) ? M : (N - produced);

        // proteger e preencher buffer (produtora insere do índice 0..take-1)
        sem_wait(&sem_mutex);
        buf_in = 0; buf_out = 0; buf_count = take;
        for (int i = 0; i < take; ++i) {
            buffer[i] = next++;
        }
        sem_post(&sem_mutex);

        // liberar contagem de itens para consumidores (postar 'take' vezes)
        for (int i = 0; i < take; ++i) sem_post(&sem_items);

        produced += take;
    }

    // enviar sentinelas (-1) para cada consumidora para que elas terminem.
    // Podem ser enviados em lotes respeitando a regra: produtor só insere quando buffer vazio.
    int sentinels_left = C;
    while (sentinels_left > 0) {
        sem_wait(&sem_buffer_empty);
        int take = (sentinels_left >= M) ? M : sentinels_left;

        sem_wait(&sem_mutex);
        buf_in = 0; buf_out = 0; buf_count = take;
        for (int i = 0; i < take; ++i) buffer[i] = -1; // sentinela
        sem_post(&sem_mutex);

        for (int i = 0; i < take; ++i) sem_post(&sem_items);
        sentinels_left -= take;
    }

    return NULL;
}

// consumidor: retira um item por vez e testa primalidade; finaliza ao ver sentinela -1
void *consumidor(void *arg) {
    int id = *(int*)arg;
    int contador = 0;

    while (1) {
        sem_wait(&sem_items);          // aguarda item disponível

        // retirar item (exclusão mútua)
        sem_wait(&sem_mutex);
        ll val = buffer[buf_out];
        buf_out = (buf_out + 1) % M;
        buf_count--;
        // se o buffer ficou vazio, sinaliza a produtora
        if (buf_count == 0) sem_post(&sem_buffer_empty);
        sem_post(&sem_mutex);

        // se for sentinela, termina
        if (val == -1) break;

        // testar primalidade (fora da região crítica)
        if (ehPrimo(val)) contador++;

        // não é necessário proteger o acesso ao array primos_por_consumidor
        // porque só esta thread escreve em primos_por_consumidor[id]
    }

    primos_por_consumidor[id] = contador;

    // adicionar ao total - proteger com sem_total
    sem_wait(&sem_total);
    total_primos += contador;
    sem_post(&sem_total);

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s N M C\n", argv[0]);
        fprintf(stderr, " N = quantidade total de numeros (produtor gera 1..N)\n");
        fprintf(stderr, " M = tamanho do buffer (canal)\n");
        fprintf(stderr, " C = numero de consumidores\n");
        return 1;
    }

    N = atoi(argv[1]);
    M = atoi(argv[2]);
    C = atoi(argv[3]);
    if (N <= 0 || M <= 0 || C <= 0) {
        fprintf(stderr, "N, M e C devem ser positivos.\n");
        return 1;
    }

    buffer = (ll*) malloc(sizeof(ll) * M);
    primos_por_consumidor = (int*) malloc(sizeof(int) * C);
    for (int i = 0; i < C; ++i) primos_por_consumidor[i] = 0;

    // inicializar semaforos
    sem_init(&sem_items, 0, 0);         // nenhum item inicialmente
    sem_init(&sem_mutex, 0, 1);         // exclusao mútua (binario)
    sem_init(&sem_buffer_empty, 0, 1);  // buffer inicialmente vazio -> produtora pode inserir
    sem_init(&sem_total, 0, 1);         // protecao para atualizar total_primos

    pthread_t prod;
    pthread_t *cons = malloc(sizeof(pthread_t) * C);
    int *ids = malloc(sizeof(int) * C);

    // criar consumidores
    for (int i = 0; i < C; ++i) {
        ids[i] = i;
        if (pthread_create(&cons[i], NULL, consumidor, &ids[i]) != 0) {
            perror("pthread_create consumidor");
            exit(1);
        }
    }

    // criar produtor
    if (pthread_create(&prod, NULL, produtor, NULL) != 0) {
        perror("pthread_create produtor");
        exit(1);
    }

    // aguardar produtor
    pthread_join(prod, NULL);

    // aguardar consumidores
    for (int i = 0; i < C; ++i) pthread_join(cons[i], NULL);

    // determinar vencedora
    int best_id = 0;
    for (int i = 1; i < C; ++i) {
        if (primos_por_consumidor[i] > primos_por_consumidor[best_id]) best_id = i;
    }

    printf("Total de primos encontrados: %d\n", total_primos);
    printf("Consumidor vencedor: id=%d com %d primos.\n", best_id, primos_por_consumidor[best_id]);

    // limpar
    sem_destroy(&sem_items);
    sem_destroy(&sem_mutex);
    sem_destroy(&sem_buffer_empty);
    sem_destroy(&sem_total);
    free(buffer);
    free(cons);
    free(ids);
    free(primos_por_consumidor);

    return 0;
}