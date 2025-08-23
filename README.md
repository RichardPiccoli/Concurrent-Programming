# Concurrent-Programming
Repository of Concurrent Programming

Atividade 4 – Produto Interno Concorrente
Descrição

Esta atividade implementa uma solução concorrente em C com Pthreads para calcular o produto interno de dois vetores de números reais.
O projeto contém dois programas:

gera_vet_prod.c → gera dois vetores aleatórios de dimensão N, grava em arquivo binário junto com o resultado sequencial.

prod_interno_conc.c → lê o arquivo gerado, calcula o produto interno de forma concorrente utilizando T threads, compara com o valor sequencial e mede o tempo de execução.

Compilação
gcc -o gera_vet_prod gera_vet_prod.c -lm
gcc -o prod_interno_conc prod_interno_conc.c -lpthread -lm

Execução

Gerar arquivos de teste:

./gera_vet_prod <N> <arquivo_saida>


Exemplo:

./gera_vet_prod 100000 vet100k.bin


Executar versão concorrente:

./prod_interno_conc <arquivo> <num_threads>


Exemplo:

./prod_interno_conc vet100k.bin 4

Experimentos realizados

Máquina utilizada: 6 núcleos físicos, 12 processadores lógicos.

Foram testados vetores com dimensões de 1k, 10k, 100k e 1M elementos.

Foram usados 1, 2, 4, 8 e 12 threads.

O tempo reportado é a média de 5 execuções.

A variação relativa foi sempre próxima de zero (≈1e-15).

Exemplo de resultado (resumido):

N (dimensão)	Threads	Tempo médio (s)	Variação relativa
1k	1	2.2e-04	0.0e+00
1k	4	3.6e-04	-2.7e-16
100k	1	5.0e-04	0.0e+00
100k	4	4.0e-04	-4.8e-15
1M	1	2.7e-03	0.0e+00
1M	4	9.0e-04	-1.1e-14
Conclusão

Os resultados mostram que:

Para vetores pequenos (1k, 10k), o overhead da criação de threads supera o ganho de paralelismo, resultando em tempos iguais ou até maiores que a versão sequencial.

Para vetores grandes (100k e 1M), o uso de múltiplas threads reduz significativamente o tempo de execução, especialmente entre 2 e 4 threads.

A partir de 8 threads, o ganho de desempenho tende a se estabilizar ou até regredir devido à contenção de recursos e overhead de sincronização.

A variação relativa foi sempre nula ou próxima de zero, confirmando a corretude do programa concorrente.
