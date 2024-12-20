#include "../common/globals.h"
#include "interface.h"
#include <pthread.h>
#include <semaphore.h>

void entra_salao_padawan(int id) {
  printf("Padawan %d está aguardando para entrar no salão\n", id);
  sem_wait(&sem_padawan_entrada);
  printf("Padawan %d entrou no salão para realizar os testes\n", id);

  sem_wait(&sem_vagas_padawans_utilizadas);

  insere_padawan_na_fila(id);
  padawans_no_salao += 1;

  sem_post(&sem_vagas_padawans_utilizadas);
}

void cumprimenta_mestres_avaliadores(int id) {
  sem_wait(&sem_cumprimento_mestres);
  printf("Padawan %d está cumprimentando os mestres avaliadores\n", id);
  printf("Padawan %d cumprimentou os mestres avaliadores\n", id);
  sem_post(&sem_cumprimento_mestres);
}

void aguarda_avaliacao(int id) {
  printf("Padawan %d está aguardando a avaliação\n", id);
  sem_wait(&sem_padawans[id]);
}

void realiza_avaliacao(int id) {
  printf("Padawan %d está realizando a avaliação\n", id);
  sleep(1);
  printf("Padawan %d realizou a avaliação\n", id);
  sem_post(&sem_padawans_output[id]);
}

int aguarda_resultado(int id) {
  printf("Padawan %d está aguardando o resultado da avaliação\n", id);
  sem_wait(&sem_padawans[id]);
  printf("Padawan %d recebeu o resultado da avaliação\n", id);
  sem_post(&sem_padawans_output[id]);
  return resultado_padawans[id];
}

void aguarda_corte_tranca(int id) {
  printf("Padawan %d está aguardando o corte de trança\n", id);
  sem_wait(&sem_padawans[id]);
  printf("Padawan %d recebeu o corte de trança\n", id);
  sem_post(&sem_padawans_output[id]);
}

void cumprimenta_Yoda(int id) {
  printf("Padawan %d está cumprimentando Yoda\n", id);
  sem_wait(&sem_padawans[id]);
  printf("Padawan %d cumprimentou Yoda\n", id);
  sem_post(&sem_padawans_output[id]);
}

void sai_salao_padawan(int id) {
  sem_wait(&sem_padawans_restantes);
  padawans_restantes -= 1;
  printf("Padawan %d está saindo do salão\n", id);
  sem_post(&sem_padawans_restantes);
}

void *padawan(void *args) {
  int id = *((int *)args);

  entra_salao_padawan(id);
  cumprimenta_mestres_avaliadores(id);
  aguarda_avaliacao(id);
  realiza_avaliacao(id);
  int resultado = aguarda_resultado(id);
  if (resultado == 1) {
    aguarda_corte_tranca(id);
  } else {
    cumprimenta_Yoda(id);
  }

  sai_salao_padawan(id);

  pthread_exit(NULL);
}
