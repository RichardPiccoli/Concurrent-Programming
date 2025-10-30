#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/md5.h> // Importa a biblioteca MD5
#include <pthread.h>
#include <time.h> // Necessário para clock_gettime

#define MAX_PASSWORD_LENGTH 10
#define DIGEST_LENGTH MD5_DIGEST_LENGTH
#define ALPHABET_SIZE 62

const char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

// --- Variáveis de Sincronização e Estado Global ---
pthread_mutex_t mutex_found_flag;
int password_found = 0;
char found_password[MAX_PASSWORD_LENGTH + 1];

// --- Estrutura de Dados para as Threads ---
typedef struct {
    unsigned char target_hash[DIGEST_LENGTH];
    int len;
    long long start_index;
    long long end_index;
} ThreadData;

// --- Funções de Utilidade ---
void hex_to_bytes(const char *hex_string, unsigned char *bytes) {
    for (int i = 0; i < DIGEST_LENGTH; i++) {
        sscanf(hex_string + 2 * i, "%2hhx", &bytes[i]);
    }
}

// --- Função Executada pelas Threads (Função de Trabalho) ---
void *search_range(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    char password[MAX_PASSWORD_LENGTH + 1];
    unsigned char hash[DIGEST_LENGTH];

    // Define o terminador da string
    password[data->len] = '\0';

    for (long long i = data->start_index; i < data->end_index; i++) {
        // Verifica a flag global para sair se a senha já foi encontrada por outra thread
        if (password_found) {
            return NULL;
        }
        
        long long temp_i = i;
        
        // Gera a senha
        for (int j = data->len - 1; j >= 0; j--) {
            password[j] = alphabet[temp_i % ALPHABET_SIZE];
            temp_i /= ALPHABET_SIZE;
        }

        // Calcula e compara o hash
        MD5((unsigned char *)password, strlen(password), hash);
        if (memcmp(hash, data->target_hash, DIGEST_LENGTH) == 0) {
            // Acesso à variável global é uma seção crítica
            pthread_mutex_lock(&mutex_found_flag);
            if (!password_found) { // Condição de corrida: apenas a primeira thread vence
                password_found = 1;
                strcpy(found_password, password);
            }
            pthread_mutex_unlock(&mutex_found_flag);
            return NULL; // Senha encontrada
        }
    }
    
    return NULL; // Fim do intervalo, senha não encontrada aqui
}

// --- Função Principal ---
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

    // Inicializa o mutex
    pthread_mutex_init(&mutex_found_flag, NULL);

    printf("Iniciando busca concorrente com %d threads.\n", num_threads);
    
    // VARIÁVEIS PARA MEDIR O TEMPO REAL (WALL-CLOCK)
    struct timespec start_time, end_time; 
    
    // INICIA O CRONÔMETRO
    clock_gettime(CLOCK_MONOTONIC, &start_time); 

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    // Lógica para dividir o trabalho por comprimento de senha
    for (int len = 1; len <= MAX_PASSWORD_LENGTH; len++) {
        long long num_combinations = 1;
        for (int i = 0; i < len; i++) {
            num_combinations *= ALPHABET_SIZE;
        }

        long long combinations_per_thread = num_combinations / num_threads;
        long long remainder = num_combinations % num_threads;

        long long start_index = 0;
        
        printf("Tentando senhas de comprimento %d...\n", len);

        for (int i = 0; i < num_threads; i++) {
            thread_data[i].len = len;
            memcpy(thread_data[i].target_hash, target_hash_bytes, DIGEST_LENGTH);
            thread_data[i].start_index = start_index;

            long long end_index = start_index + combinations_per_thread;
            if (i < remainder) { // Distribui o resto do trabalho
                end_index++;
            }
            thread_data[i].end_index = end_index;

            start_index = end_index;

            pthread_create(&threads[i], NULL, search_range, &thread_data[i]);
        }
        
        // Espera todas as threads deste comprimento terminarem
        for (int i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
        }

        if (password_found) {
            break;
        }
    }

    // FINALIZA O CRONÔMETRO
    clock_gettime(CLOCK_MONOTONIC, &end_time); 
    
    // CALCULA A DIFERENÇA EM SEGUNDOS
    long seconds = end_time.tv_sec - start_time.tv_sec;
    long nanoseconds = end_time.tv_nsec - start_time.tv_nsec;

    if (nanoseconds < 0) {
        seconds--;
        nanoseconds += 1000000000;
    }
    
    double time_spent = (double)seconds + (double)nanoseconds / 1000000000.0;

    if (password_found) {
        printf("\n=> Senha encontrada: %s\n", found_password);
    } else {
        printf("\n=> Senha nao encontrada no espaco de busca.\n");
    }
    printf("Tempo de execucao concorrente: %f segundos\n", time_spent);

    pthread_mutex_destroy(&mutex_found_flag);

    return 0;
}