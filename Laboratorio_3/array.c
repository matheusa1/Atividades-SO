// Autor: Matheus Santos de Andrade, João Vitor Girotto , Victórya Carolina Guimarães da Luz
// Data: 2024-10-28

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Faça um programa com N threads que localiza um valor em um vetor de inteiros. O espaço de busca no
// vetor deve ser distribuído para as N threads.
typedef struct ThreadData {
    int* array;
    int start;
    int end;
    int searchValue;
  } ThreadData;

// Funções
int* generateArray(int size);
void printArray(int size, int* array);
void* searchValueFn(void* data);
ThreadData* createStruct();

int main(int argc, char* argv[]) {
  if (argc < 3) {
    printf("Uso: %s <tamanho do vetor> <quantidade_threads>\n", argv[0]);
    return 1;
  }

  int searchValue;

  int threadCount = atoi(argv[2]);
  int arraySize = atoi(argv[1]);

  int* array = generateArray(arraySize);
  printArray(arraySize, array);

  printf("Digite o valor a ser procurado: ");
  scanf("%d", &searchValue);

  int sizePartPerThread;

  if(arraySize > threadCount) {
      sizePartPerThread = arraySize / threadCount;
  } else {
      sizePartPerThread = 1;
      threadCount = arraySize;
  }

  // Criação das threads
  for(int i = 0; i < threadCount; i++) {
    pthread_t thread;

    ThreadData* data = createStruct();

    int start = i * sizePartPerThread;
    int end = (i == threadCount - 1) ? arraySize - 1 : (start + sizePartPerThread - 1);

    data->array = array;
    data->start = start;
    data->end = end;
    data->searchValue = searchValue;

    pthread_create(&thread, NULL, searchValueFn, (void*) data);
  }

  pthread_exit(NULL);
}

// Função que aloca memória para a struct ThreadData.
ThreadData* createStruct() {
  ThreadData* data = (ThreadData*) malloc(sizeof(ThreadData));
  return data;
}

// Função que gera um vetor de inteiros de tamanho size com valores aleatórios.
int* generateArray(int size) {
  int* array = (int*) malloc(size * sizeof(int));
  for(int i = 0; i < size; i++) {
    array[i] = rand() % 10;
  }
  return array; 
}

// Função que imprime um vetor de inteiros.
void printArray(int size, int* array) {
  printf("Array: [");
  for(int i = 0; i < size; i++) {
    printf("%d, ", array[i]);
  }
  printf("\b\b]\n");
}

// Função que procura um valor em um vetor de inteiros.
void* searchValueFn(void* data) {
  ThreadData* threadData = (ThreadData*) data;
  int foundValue = 0;

  for(int i = threadData->start; i <= threadData->end; i++) {
    if(threadData->array[i] == threadData->searchValue) {
      printf("Valor encontrado na posição %d - thread [%d...%d]\n", i, threadData->start, threadData->end);
      foundValue = 1;
    }
  }

  if(!foundValue) {
    printf("Valor não encontrado - thread [%d...%d]\n", threadData->start, threadData->end);
  }

  free(threadData);

  pthread_exit(NULL);
}