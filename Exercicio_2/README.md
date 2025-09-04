Atividade 5 – Programação Concorrente

Objetivo

Retomar o exercício da aula e implementar um requisito condicional de execução das threads.
O programa foi modificado para que:

As threads de soma pausam quando o valor da variável compartilhada soma atinge um múltiplo de 1000.

A thread extra imprime esse valor.

Após a impressão, as threads de soma são liberadas para continuar a execução.

Arquivos

exercicio2.c → Implementação da Atividade 5

Compilação

Compile usando pthread:

gcc -o exercicio2 exercicio2.c -lpthread

Execução

Rode o programa informando o número de threads de soma:

./exercicio2 <nthreads>


Exemplo com 4 threads de soma:

./exercicio2 4

Exemplo de saída (trecho)
Thread 0 esta executando...
Thread 1 esta executando...
Thread 2 esta executando...
Thread 3 esta executando...
Extra esta executando...
soma = 1000
soma = 2000
soma = 3000
...
soma = 400000
Extra terminou!
Thread 0 terminou!
Thread 1 terminou!
Thread 2 terminou!
Thread 3 terminou!
Valor final de soma = 400000

Conclusão

Com o uso de variáveis de condição (pthread_cond_t), foi possível sincronizar as threads de soma com a thread extra. Dessa forma, garantimos que:

Nenhum múltiplo de 1000 é perdido.

A thread extra é sempre responsável por imprimir.

As threads de soma continuam a execução apenas após a impressão.