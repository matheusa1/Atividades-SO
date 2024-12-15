#include "globals.h"
#include <pthread.h>
#include <semaphore.h>

// Definição dos semáforos
sem_t sem_publico_entrada;
sem_t sem_padawan_entrada;
sem_t sem_padawan_pronto;
sem_t sem_avaliacao_andamento;
sem_t sem_avaliacao_resultado;
sem_t sem_aguarda_corte_trança;
sem_t sem_padawans_restantes;
sem_t sem_padawan_saiu;
sem_t sem_vagas_publico_utilizadas;

// // Definição das variáveis globais
int resultado_avaliacao = 0;
int entrada_disponivel = 0;
int vagas_publico_utilizadas = 0;
int padawans_restantes;

void inicia_semaforos() {
  sem_init(&sem_publico_entrada, 0, 0);
  sem_init(&sem_padawan_entrada, 0, 0);
  sem_init(&sem_padawan_pronto, 0, 0);
  sem_init(&sem_avaliacao_andamento, 0, 0);
  sem_init(&sem_avaliacao_resultado, 0, 0);
  sem_init(&sem_aguarda_corte_trança, 0, 0);
  sem_init(&sem_publico_entrada, 0, 0);
  sem_init(&sem_padawan_saiu, 0, 0);
  sem_init(&sem_vagas_publico_utilizadas, 0, 1);
}

void destroi_semaforos() {
  sem_destroy(&sem_publico_entrada);
  sem_destroy(&sem_padawan_entrada);
  sem_destroy(&sem_padawan_pronto);
  sem_destroy(&sem_avaliacao_andamento);
  sem_destroy(&sem_avaliacao_resultado);
  sem_destroy(&sem_aguarda_corte_trança);
  sem_destroy(&sem_publico_entrada);
  sem_destroy(&sem_padawan_saiu);
  sem_destroy(&sem_vagas_publico_utilizadas);
}
