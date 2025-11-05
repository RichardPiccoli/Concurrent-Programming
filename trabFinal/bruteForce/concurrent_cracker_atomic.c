// concurrent_cracker_atomic.c
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>

#define MAX_PASSWORD_LENGTH 10
#define DIGEST_LENGTH MD5_DIGEST_LENGTH
#define ALPHABET_SIZE 62

const char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

// --- Estado global ---
atomic_int password_found = 0; // 0 = não, 1 = sim
char found_password[MAX_PASSWORD_LENGTH + 1];

// --- Estrutura para threads ---
typedef struct {
    unsigned char target_hash[DIGEST_LENGTH];
    int len;
    long long start_index;
    long long end_index;
} ThreadData;

void hex_to_bytes(const char *hex_string, unsigned char *bytes) {
    for (int i = 0; i < DIGEST_LENGTH; i++) {
        sscanf(hex_string + 2 * i, "%2hhx", &bytes[i]);
    }
}

void *search_range(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    char password[MAX_PASSWORD_LENGTH + 1];
    unsigned char hash[DIGEST_LENGTH];

    password[data->len] = '\0';

    for (long long i = data->start_index; i < data->end_index; i++) {
        // Leitura relaxada da flag (baixa latência)
        if (atomic_load_explicit(&password_found, memory_order_relaxed)) {
            return NULL;
        }

        long long temp_i = i;
        for (int j = data->len - 1; j >= 0; j--) {
            password[j] = alphabet[temp_i % ALPHABET_SIZE];
            temp_i /= ALPHABET_SIZE;
        }

        MD5((unsigned char *)password, strlen(password), hash);
        if (memcmp(hash, data->target_hash, DIGEST_LENGTH) == 0) {
            // Tenta marcar a flag atomicamente: apenas uma thread vencerá
            int expected = 0;
            if (atomic_compare_exchange_strong_explicit(
                    &password_found, &expected, 1,
                    memory_order_acq_rel, memory_order_relaxed)) {
                // Só a thread que conseguiu trocar 0->1 escreve a senha
                strcpy(found_password, password);
            }
            // Seja que venceu ou não, saia
            return NULL;
        }
    }

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <hash_md5> <num_threads>\n", argv[0]);
        return 1;
    }

    char *hex_hash = argv[1];
    int num_threads = atoi(argv[2]);
    unsigned char target_hash_bytes[DIGEST_LENGTH];

    if (strlen(hex_hash) != DIGEST_LENGTH * 2) {
        fprintf(stderr, "Erro: O hash MD5 deve ter 32 caracteres.\n");
        return 1;
    }
    if (num_threads <= 0) {
        fprintf(stderr, "Erro: O número de threads deve ser positivo.\n");
        return 1;
    }

    hex_to_bytes(hex_hash, target_hash_bytes);

    printf("Iniciando busca concorrente com %d threads.\n", num_threads);

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    ThreadData *thread_data = malloc(sizeof(ThreadData) * num_threads);

    for (int len = 1; len <= MAX_PASSWORD_LENGTH; len++) {
        // Se já foi encontrado, quebra
        if (atomic_load_explicit(&password_found, memory_order_relaxed)) break;

        long long num_combinations = 1;
        for (int i = 0; i < len; i++) num_combinations *= ALPHABET_SIZE;

        long long combinations_per_thread = num_combinations / num_threads;
        long long remainder = num_combinations % num_threads;
        long long start_index = 0;

        printf("Tentando senhas de comprimento %d...\n", len);

        for (int i = 0; i < num_threads; i++) {
            thread_data[i].len = len;
            memcpy(thread_data[i].target_hash, target_hash_bytes, DIGEST_LENGTH);
            thread_data[i].start_index = start_index;

            long long end_index = start_index + combinations_per_thread;
            if (i < remainder) end_index++;
            thread_data[i].end_index = end_index;
            start_index = end_index;

            pthread_create(&threads[i], NULL, search_range, &thread_data[i]);
        }

        for (int i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
        }

        if (atomic_load_explicit(&password_found, memory_order_relaxed)) break;
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    long seconds = end_time.tv_sec - start_time.tv_sec;
    long nanoseconds = end_time.tv_nsec - start_time.tv_nsec;
    if (nanoseconds < 0) { seconds--; nanoseconds += 1000000000; }
    double time_spent = (double)seconds + (double)nanoseconds / 1e9;

    if (atomic_load_explicit(&password_found, memory_order_relaxed)) {
        printf("\n=> Senha encontrada: %s\n", found_password);
    } else {
        printf("\n=> Senha nao encontrada no espaco de busca.\n");
    }
    printf("Tempo de execucao concorrente: %f segundos\n", time_spent);

    free(threads);
    free(thread_data);
    return 0;
}
