#include "globals.h"
#include <pthread.h>
#include <semaphore.h>

// Definição dos semáforos
sem_t sem_publico_entrada;
sem_t sem_padawan_entrada;
sem_t sem_vagas_publico_utilizadas;
sem_t sem_vagas_padawans_utilizadas;
sem_t sem_cumprimento_mestres;
sem_t sem_padawans[NUM_PADAWAN];
sem_t sem_padawans_output[NUM_PADAWAN];
sem_t sem_avaliacao_andamento;
sem_t sem_padawans_restantes;

// // Definição das variáveis globais
int entrada_disponivel = 0;
int vagas_publico_utilizadas = 0;
int padawans_no_salao = 0;
int fila_padawans[NUM_PADAWAN];
int resultado_padawans[NUM_PADAWAN];
int padawans_aprovados = 0;
int padawans_restantes = NUM_PADAWAN;

void inicia_semaforos() {
  sem_init(&sem_publico_entrada, 0, 0);
  sem_init(&sem_padawan_entrada, 0, 0);
  sem_init(&sem_vagas_publico_utilizadas, 0, 1);
  sem_init(&sem_vagas_padawans_utilizadas, 0, 1);
  sem_init(&sem_cumprimento_mestres, 0, 1);
  sem_init(&sem_avaliacao_andamento, 0, 0);
  sem_init(&sem_padawans_restantes, 0, 1);

  for (int i = 0; i < NUM_PADAWAN; i++) {
    sem_init(&sem_padawans[i], 0, 0);
  }

  for (int i = 0; i < NUM_PADAWAN; i++) {
    sem_init(&sem_padawans_output[i], 0, 0);
  }
}

void destroi_semaforos() {
  sem_destroy(&sem_publico_entrada);
  sem_destroy(&sem_padawan_entrada);
  sem_destroy(&sem_vagas_publico_utilizadas);
  sem_destroy(&sem_vagas_padawans_utilizadas);
  sem_destroy(&sem_cumprimento_mestres);
  sem_destroy(&sem_avaliacao_andamento);
  sem_destroy(&sem_padawans_restantes);

  for (int i = 0; i < NUM_PADAWAN; i++) {
    sem_destroy(&sem_padawans[i]);
  }

  for (int i = 0; i < NUM_PADAWAN; i++) {
    sem_destroy(&sem_padawans_output[i]);
  }
}

void insere_padawan_na_fila(int id) { fila_padawans[padawans_no_salao] = id; }

void remove_padawan_da_fila() {
  for (int i = 0; i < NUM_PADAWAN - 1; i++) {
    fila_padawans[i] = fila_padawans[i + 1];
  }
}
