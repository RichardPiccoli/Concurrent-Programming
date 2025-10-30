#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/md5.h> // Importa a biblioteca MD5
#include <time.h>

#define MAX_PASSWORD_LENGTH 10
#define DIGEST_LENGTH MD5_DIGEST_LENGTH
#define ALPHABET_SIZE 62

const char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

// Função de utilidade para converter hash hexadecimal para bytes
void hex_to_bytes(const char *hex_string, unsigned char *bytes) {
    for (int i = 0; i < DIGEST_LENGTH; i++) {
        sscanf(hex_string + 2 * i, "%2hhx", &bytes[i]);
    }
}

// Estrutura de dados para a busca
typedef struct {
    unsigned char target_hash[DIGEST_LENGTH];
    int len;
    long long start_index;
    long long end_index;
} SearchData;

// A função de busca agora retorna um int para sinalizar o resultado
int search_range(void *arg) {
    SearchData *data = (SearchData *)arg;
    char password[MAX_PASSWORD_LENGTH + 1];
    unsigned char hash[DIGEST_LENGTH];
    
    password[data->len] = '\0';

    for (long long i = data->start_index; i < data->end_index; i++) {
        long long temp_i = i;
        
        // Gera a senha
        for (int j = data->len - 1; j >= 0; j--) {
            password[j] = alphabet[temp_i % ALPHABET_SIZE];
            temp_i /= ALPHABET_SIZE;
        }

        // Calcula e compara o hash
        MD5((unsigned char *)password, strlen(password), hash);
        if (memcmp(hash, data->target_hash, DIGEST_LENGTH) == 0) {
            //printf("found: %s\n", password);
            printf("\n=> Senha encontrada: %s\n", password);
            return 1; // Senha encontrada
        }
    }
    
    return 0; // Senha não encontrada neste intervalo
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <hash_md5>\n", argv[0]);
        return 1;
    }

    char *hex_hash = argv[1];
    unsigned char target_hash_bytes[DIGEST_LENGTH];
    int found = 0;

    if (strlen(hex_hash) != DIGEST_LENGTH * 2) {
        fprintf(stderr, "Erro: O hash MD5 deve ter 32 caracteres.\n");
        return 1;
    }

    hex_to_bytes(hex_hash, target_hash_bytes);

    printf("Iniciando busca sequencial.\n");
    clock_t start_time = clock();

    for (int len = 1; len <= MAX_PASSWORD_LENGTH; len++) {
        long long num_combinations = 1;
        for (int i = 0; i < len; i++) {
            num_combinations *= ALPHABET_SIZE;
        }

        SearchData data;
        memcpy(data.target_hash, target_hash_bytes, DIGEST_LENGTH);
        data.len = len;
        data.start_index = 0;
        data.end_index = num_combinations;

        printf("Tentando senhas de comprimento %d...\n", len);
        if (search_range(&data) == 1) {
            found = 1;
            break; // Sai do loop principal
        }
    }

    clock_t end_time = clock();
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    if (found) {
        //printf("\n=> Senha encontrada: %s\n", password);
    } else {
        printf("\n=> Senha nao encontrada no espaco de busca.\n");
    }
    printf("Tempo de execucao sequencial: %f segundos\n", time_spent);

    return 0;
}