// Autor: Matheus Santos de Andrade
// Data: 29/11/2024
// Descrição: Programa que realiza a soma de dois vetores de inteiros de mesmo
// tamanho, utilizando processos filhos para dividir o trabalho. O número de
// elementos do vetor e o número de processos filhos são passados pelo usuário.

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#endif

int main() {
  int numElementos, numProcessos;
  printf("Digite o número de elementos do vetor: ");
  scanf("%d", &numElementos);
  printf("Digite o número de processos filhos: ");
  scanf("%d", &numProcessos);

  int tamanhoParte = numElementos / numProcessos;
  int resto = numElementos % numProcessos;

  // Alocar memória compartilhada
  int *V1 = mmap(NULL, numElementos * sizeof(int), PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  int *V2 = mmap(NULL, numElementos * sizeof(int), PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  int *V3 = mmap(NULL, numElementos * sizeof(int), PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  int *sinalizacao =
      mmap(NULL, numProcessos * sizeof(int), PROT_READ | PROT_WRITE,
           MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  // Inicializar vetores V1 e V2
  for (int i = 0; i < numElementos; i++) {
    V1[i] = rand() % 10;
    V2[i] = rand() % 10;
  }

  // Imprime os vetores V1 e V2
  printf("Vetor V1:\n");
  for (int i = 0; i < numElementos; i++) {
    printf("%d ", V1[i]);
  }
  printf("\n");

  printf("Vetor V2:\n");
  for (int i = 0; i < numElementos; i++) {
    printf("%d ", V2[i]);
  }
  printf("\n");

  int pipes[numProcessos][2];
  pid_t pids[numProcessos];

  // Criar pipes e processos filhos
  for (int i = 0; i < numProcessos; i++) {
    pipe(pipes[i]);
    pids[i] = fork();
    if (pids[i] == 0) {
      // Filho
      close(pipes[i][1]);
      int inicio, fim;
      read(pipes[i][0], &inicio, sizeof(int));
      read(pipes[i][0], &fim, sizeof(int));
      for (int j = inicio; j < fim; j++) {
        V3[j] = V1[j] + V2[j];
      }
      sinalizacao[i] = 1;
      close(pipes[i][0]);
      munmap(V1, numElementos * sizeof(int));
      munmap(V2, numElementos * sizeof(int));
      munmap(V3, numElementos * sizeof(int));
      munmap(sinalizacao, numProcessos * sizeof(int));
      exit(0);
    } else {
      // Pai
      close(pipes[i][0]);
    }
  }

  // Pai envia intervalos para os filhos
  int inicio = 0;
  for (int i = 0; i < numProcessos; i++) {
    int fim = inicio + tamanhoParte;
    if (i == numProcessos - 1) {
      fim += resto;
    }
    write(pipes[i][1], &inicio, sizeof(int));
    write(pipes[i][1], &fim, sizeof(int));
    close(pipes[i][1]);
    inicio = fim;
  }

  // Pai aguarda os filhos terminarem
  int filhosTerminados = 0;
  while (filhosTerminados < numProcessos) {
    filhosTerminados = 0;
    for (int i = 0; i < numProcessos; i++) {
      if (sinalizacao[i] == 1) {
        filhosTerminados++;
      }
    }
  }

  // Imprime o resultado
  printf("Resultado da soma:\n");
  for (int i = 0; i < numElementos; i++) {
    printf("%d ", V3[i]);
  }
  printf("\n");

  // Limpeza
  for (int i = 0; i < numProcessos; i++) {
    wait(NULL);
  }
  munmap(V1, numElementos * sizeof(int));
  munmap(V2, numElementos * sizeof(int));
  munmap(V3, numElementos * sizeof(int));
  munmap(sinalizacao, numProcessos * sizeof(int));

  return 0;
}
