#include "../common/globals.h"
#include "interface.h"
#include <pthread.h>
#include <semaphore.h>

// --------------------------- FUNÇÕES DE AÇÃO DO PADAWAN
// ---------------------------

// Função que controla a entrada do Padawan no salão.
// Parâmetro: `id` - identificador da thread do Padawan.
// Saída: Mensagens informando quando o Padawan aguarda e entra no salão.
void entra_salao_padawan(int id) {
  printf("Padawan %d está aguardando para entrar no salão\n", id);
  // Aguarda permissão para entrar no salão
  do {
    sem_wait(&sem_padawan_entrada);
  } while (entrada_disponivel == 0);

  printf("Padawan %d entrou no salão para realizar os testes\n", id);

  // Bloqueia o acesso às vagas para atualizar a fila de Padawans no salão.
  sem_wait(&sem_vagas_padawans_utilizadas);
  // Insere o Padawan na fila e incrementa o número de Padawans no salão.
  insere_padawan_na_fila(id);
  padawans_no_salao += 1;
  // Libera o acesso às vagas.
  sem_post(&sem_vagas_padawans_utilizadas);
}

// Função que representa o cumprimento dos mestres avaliadores.
// Parâmetro: `id` - identificador da thread do Padawan.
// Saída: Mensagens informando que o Padawan está cumprimentando os mestres.
void cumprimenta_mestres_avaliadores(int id) {
  sem_wait(&sem_cumprimento_mestres);
  printf("Padawan %d está cumprimentando os mestres avaliadores\n", id);
  printf("Padawan %d cumprimentou os mestres avaliadores\n", id);
  sem_post(&sem_cumprimento_mestres);
}

// Função que faz o Padawan aguardar para ser avaliado.
// Parâmetro: `id` - identificador da thread do Padawan.
// Saída: Mensagem informando que o Padawan está aguardando a avaliação.
void aguarda_avaliacao(int id) {
  printf("Padawan %d está aguardando a avaliação\n", id);
  // Espera pela liberação para iniciar a avaliação.
  sem_wait(&sem_padawans[id]);
}

// Função que faz o Padawan realizar a avaliação.
// Parâmetro: `id` - identificador da thread do Padawan.
// Saída: Mensagens indicando o início e término da avaliação.
void realiza_avaliacao(int id) {
  printf("Padawan %d está realizando a avaliação\n", id);
  sleep(1);
  printf("Padawan %d realizou a avaliação\n", id);
  // Sinaliza que a avaaliação foi concluida.
  sem_post(&sem_padawans_output[id]);
}

// Função que faz o Padawan aguardar o resultado da avaliação.
// Parâmetro: `id` - identificador da thread do Padawan.
// Saída: Mensagens indicando que o Padawan está aguardando e recebeu o
// resultado. Retorno: Resultado da avaliação (1 - aprovado, 2 - reprovado).
int aguarda_resultado(int id) {
  printf("Padawan %d está aguardando o resultado da avaliação\n", id);
  // Aguarda o resultado da avaliação.
  sem_wait(&sem_padawans[id]);
  printf("Padawan %d recebeu o resultado da avaliação\n", id);
  // Sinaliza que recebeu o resultado.
  sem_post(&sem_padawans_output[id]);
  return resultado_padawans[id];
}

// Função que faz o Padawan aguardar o corte da trança após ser aprovado.
// Parâmetro: `id` - identificador da thread do Padawan.
// Saída: Mensagens indicando a espera e a realização do corte da trança.
void aguarda_corte_tranca(int id) {
  printf("Padawan %d está aguardando o corte de trança\n", id);
  // Aguarda o corte da trança.
  sem_wait(&sem_padawans[id]);
  printf("Padawan %d recebeu o corte de trança\n", id);
  // Sinaliza que o corte foi concluído.
  sem_post(&sem_padawans_output[id]);
}

// Função que faz o Padawan cumprimentar Yoda após ser reprovado.
// Parâmetro: `id` - identificador da thread do Padawan.
// Saída: Mensagens indicando que o Padawan cumprimenta Yoda.
void cumprimenta_Yoda(int id) {
  printf("Padawan %d está cumprimentando Yoda\n", id);
  sem_wait(&sem_padawans[id]);
  printf("Padawan %d cumprimentou Yoda\n", id);
  // Sinaliza que o cumprimento foi concluído.
  sem_post(&sem_padawans_output[id]);
}

// Função que faz o Padawan sair do salão.
// Parâmetro: `id` - identificador da thread do Padawan.
// Saída: Mensagem indicando que o Padawan está saindo do salão.
void sai_salao_padawan(int id) {
  // Bloqueia para atualizar o número de Padawans restantes.
  sem_wait(&sem_padawans_restantes);
  padawans_restantes -= 1;
  printf("Padawan %d está saindo do salão\n", id);
  // Libera após a atualização.
  sem_post(&sem_padawans_restantes);
}

// --------------------------- FUNÇÃO PRINCIPAL DA THREAD PADAWAN
// ---------------------------

// Função principal da thread do Padawan que coordena todas as etapas.
// Parâmetro: `args` - ponteiro para o identificador do Padawan.
// Saída: Mensagens de progresso em cada etapa do Padawan.

void *padawan(void *args) {
  // Obtém o ID do Padawan a partir dos argumentos.
  int id = *((int *)args);

  sleep(rand() % 10);

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
