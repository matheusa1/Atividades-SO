// Autor: Matheus Santos de Andrade
// Data: 13/12/2024
// Descrição: implementaçã do problema classico leitores/escritores usando
// semáforos

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int readersCount = 0; // Contador de leitores
int data = 0;         // Recurso compartilhado
sem_t mutex;          // Semáforo para acesso ao readersCount
sem_t writer;         // Semáforo para controlar acesso de escritores

void *readerFn(void *arg);
void *writerFn(void *arg);

int main() {
  int num_readers = 5;
  int num_writers = 2;
  pthread_t readers[num_readers], writers[num_writers];
  int id_readers[num_readers], id_writers[num_writers];

  // Inicialização dos semáforos
  sem_init(&mutex, 0, 1);
  sem_init(&writer, 0, 1);

  // Criação das threads escritores
  for (int i = 0; i < num_writers; i++) {
    id_writers[i] = i + 1;
    pthread_create(&writers[i], NULL, writerFn, &id_writers[i]);
  }

  // Criação das threads leitores
  for (int i = 0; i < num_readers; i++) {
    id_readers[i] = i + 1;
    pthread_create(&readers[i], NULL, readerFn, &id_readers[i]);
  }

  // Espera as threads terminarem
  for (int i = 0; i < num_readers; i++) {
    pthread_join(readers[i], NULL);
  }

  for (int i = 0; i < num_writers; i++) {
    pthread_join(writers[i], NULL);
  }

  // Destrói os semáforos
  sem_destroy(&mutex);
  sem_destroy(&writer);

  return 0;
}

void *writerFn(void *arg) {
  int id = *(int *)arg;
  int count = 0;
  while (count < 5) {
    // Acessa a seção crítica
    sem_wait(&writer);

    // Seção de escrita
    data = rand() % 100; // Modifica o recurso compartilhado
    printf("Escritor %d está escrevendo. Novo valor = %d\n", id, data);
    // sleep(1); // Simula tempo de escrita

    // Libera a seção crítica
    sem_post(&writer);

    count++;
    sleep(rand() % 2); // Tempo até a próxima escrita
  }

  printf("Escritor %d parou de escrever\n", id);
  pthread_exit(0);
}

void *readerFn(void *arg) {
  int id = *(int *)arg;

  int count = 0;
  while (count < 5) {
    // Entrada na seção crítica
    sem_wait(&mutex);
    readersCount++;
    if (readersCount == 1)
      sem_wait(&writer); // Bloqueia escritores
    sem_post(&mutex);

    // Seção de leitura
    printf("Leitor %d está lendo. Valor compartilhado = %d\n", id, data);
    // sleep(1); // Simula tempo de leitura

    // Saída da seção crítica
    sem_wait(&mutex);
    readersCount--;
    if (readersCount == 0)
      sem_post(&writer); // Libera escritores
    sem_post(&mutex);

    sleep(rand() % 2); // Tempo até a próxima leitura
    count++;
  }

  printf("Leitor %d parou de ler\n", id);
  pthread_exit(0);
}
