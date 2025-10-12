PRODUTOR–CONSUMIDOR COM SEMÁFOROS (VERIFICAÇÃO DE PRIMOS)

Descrição

Programa em C que implementa o padrão produtor/consumidor usando apenas semáforos POSIX (sem pthread_mutex).
Uma thread produtora gera N números consecutivos e várias threads consumidoras retiram um número por vez do buffer,
verificando se é primo. Ao final, o programa mostra o total de primos e a consumidora vencedora.


Compilação

gcc -o prodcons_primos prodcons_primos.c -pthread -lm


Execução

./prodcons_primos N M C

Parâmetros:
N = quantidade total de números a produzir (1..N)
M = tamanho do buffer (canal)
C = número de threads consumidoras

Exemplo:
./prodcons_primos 1000 10 4

Saída esperada:
Total de primos encontrados: 168
Consumidor vencedor: id=3 com 68 primos.


Proteção contra corrida de dados

- Acesso ao buffer protegido por semáforo binário (sem_mutex).
- Consumidores só retiram itens quando há itens disponíveis (sem_items).
- Produtor só insere quando buffer está vazio (sem_buffer_empty).
- Atualização de total_primos protegida por sem_total.
- Cada consumidora escreve apenas em sua própria posição no vetor de resultados.
- O uso de sentinelas (-1) garante término ordenado das threads.


Resumo

Linguagem: C
Sincronização: semáforos POSIX (sem_t)
Padrão: Produtor/Consumidor com múltiplos consumidores
Evita corrida de dados: Sim (verificado por design e via ThreadSanitizer)
Compatível com: Linux / macOS
