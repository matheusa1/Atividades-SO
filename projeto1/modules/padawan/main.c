#include "../common/globals.h"
#include "interface.h"
#include <pthread.h>
#include <semaphore.h>

void entra_salao_padawan(int id) {
  printf("Padawan %d está aguardando para entrar no salão\n", id);
  sem_wait(&sem_padawan_entrada);
  printf("Padawan %d entrou no salão para realizar os testes\n", id);
}

void cumprimenta_mestres_avaliadores(int id) {
  printf("Padawan %d está cumprimentando os mestres avaliadores\n", id);
  sleep(1);
  printf("Padawan %d terminou de cumprimentar os mestres avaliadores\n", id);
}

void pronto_para_avaliacao(int id) {
  printf("Padawan %d está pronto para ser avaliado\n", id);
  sem_post(&sem_padawan_pronto);
}

void aguarda_avaliacao(int id) {
  printf("Padawan %d está aguardando a avaliação\n", id);
  sem_wait(&sem_avaliacao_andamento);
  printf("Padawan %d recebeu a avaliação\n", id);
}

void realiza_avaliacao(int id) {
  printf("Padawan %d está realizando a avaliação\n", id);
  sleep(1);
  printf("Padawan %d terminou a avaliação\n", id);
}

void aguarda_resultado(int id) {
  printf("Padawan %d está esperando o resultado\n", id);
  sem_post(&sem_padawan_pronto);
  sem_wait(&sem_avaliacao_resultado);
}

void comprimenta_yoda(int id) {
  printf("Padawan %d está cumprimentando Yoda\n", id);
  sleep(1);
  printf("Padawan %d terminou de cumprimentar Yoda\n", id);
  sem_post(&sem_padawan_pronto);
}

void sai_salao_padawan(int id) {
  printf("Padawan %d está saindo do salão\n", id);
  sem_post(&sem_padawan_saiu);
}

void aguarda_corte_tranca(int id) {
  printf("Padawan %d está aguardando o corte da trança\n", id);
  sem_post(&sem_aguarda_corte_trança);
  sem_wait(&sem_padawan_pronto);
  printf("Padawan %d foi promovido a Jedi 🙏🙏🙏 🖐️ 🌬️ 🌀\n", id);
}

void *padawan(void *args) {
  int id = *((int *)args);

  entra_salao_padawan(id);
  cumprimenta_mestres_avaliadores(id);
  pronto_para_avaliacao(id);
  aguarda_avaliacao(id);
  realiza_avaliacao(id);
  aguarda_resultado(id);

  if (resultado_avaliacao == 2) {
    comprimenta_yoda(id);
    sai_salao_padawan(id);
  } else {
    aguarda_corte_tranca(id);
    sai_salao_padawan(id);
  }

  pthread_exit(NULL);
}
