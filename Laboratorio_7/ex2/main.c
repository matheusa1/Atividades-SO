// Autor: Matheus Santos de Andrade
// Data: 13/12/2024
// Descrição: implementaçã do problema classico jantar dos filósofos usando uma
// implementação de monitores

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define N 5 // Número de filósofos

// Estados possíveis de um filósofo
typedef enum { PENSANDO, FAMINTO, COMENDO } state_t;

state_t state[N];       // Estado de cada filósofo
pthread_mutex_t mutex;  // Mutex para controlar acesso à região crítica
pthread_cond_t cond[N]; // Condição para cada filósofo

void *philosopher(void *num);
void takeChopsticks(int i);
void putChopsticks(int i);
void test(int i);

int main() {
  int i, a[N];
  pthread_t tid[N];

  // Inicializa o mutex e as condições
  pthread_mutex_init(&mutex, NULL);
  for (i = 0; i < N; i++) {
    pthread_cond_init(&cond[i], NULL);
    state[i] = PENSANDO; // Inicialmente, todos estão pensando
  }

  // Cria as threads dos filósofos
  for (i = 0; i < N; i++) {
    a[i] = i;
    pthread_create(&tid[i], NULL, philosopher, &a[i]);
  }

  // Aguarda as threads terminarem (neste caso, nunca)
  for (i = 0; i < N; i++) {
    pthread_join(tid[i], NULL);
  }

  return 0;
}

void *philosopher(void *num) {
  int i = *(int *)num;

  int count = 0;

  while (count < 5) {
    // Filósofo está pensando
    printf("Filósofo %d está PENSANDO.\n", i);
    sleep(1); // Simula o tempo pensando

    takeChopsticks(i); // Tenta pegar os garfos

    // Filósofo está comendo
    printf("Filósofo %d está COMENDO.\n", i);
    sleep(1); // Simula o tempo comendo

    putChopsticks(i); // Devolve os garfos

    count++;
  }

  printf("Filósofo %d saiu\n", i);
}

void takeChopsticks(int i) {
  pthread_mutex_lock(&mutex);

  state[i] = FAMINTO;
  test(i);

  // Se não puder comer, espera na condição
  while (state[i] != COMENDO) {
    pthread_cond_wait(&cond[i], &mutex);
  }

  pthread_mutex_unlock(&mutex);
}

void putChopsticks(int i) {
  pthread_mutex_lock(&mutex);

  state[i] = PENSANDO;

  // Testa os filósofos adjacentes
  test((i + N - 1) % N); // Filósofo à esquerda
  test((i + 1) % N);     // Filósofo à direita

  pthread_mutex_unlock(&mutex);
}

void test(int i) {
  // Se o filósofo está faminto e os vizinhos não estão comendo
  if (state[i] == FAMINTO && state[(i + N - 1) % N] != COMENDO &&
      state[(i + 1) % N] != COMENDO) {

    state[i] = COMENDO;
    pthread_cond_signal(&cond[i]); // Permite que o filósofo coma
  }
}
