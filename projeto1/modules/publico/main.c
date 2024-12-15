#include "../common/globals.h"
#include <pthread.h>
#include <semaphore.h>

void entra_salao_publico(int id) {
  printf("Espectador %d está aguardando para entrar no salão\n", id);
  sem_wait(&sem_publico_entrada);
  sem_wait(&sem_vagas_publico_utilizadas);
  vagas_publico_utilizadas++;
  printf("Espectador %d entrou no salão para assistir os testes\n", id);
  sem_post(&sem_vagas_publico_utilizadas);
}

void sai_salao(int id) {
  sem_wait(&sem_vagas_publico_utilizadas);
  vagas_publico_utilizadas--;
  printf("Espectador %d saiu do salão\n", id);
  sem_post(&sem_vagas_publico_utilizadas);
}

void assiste_teste(int id) {
  sem_wait(&sem_avaliacao_andamento);
  printf("Espectador %d está assistindo teste\n", id);
  sleep(rand() % 10);
}

void *publico(void *arg) {
  int id = *(int *)arg;

  while (padawans_restantes > 0) {
    entra_salao_publico(id);
    assiste_teste(id);
    sai_salao(id);
  }
}
