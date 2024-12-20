#include "../common/globals.h"
#include <pthread.h>
#include <semaphore.h>

// --------------------------- FUNÇÕES DE AÇÃO DO PUBLICO ---------------------------

// Função que permite um espectador entrar no salão.
// Parâmetro: Id do Espectador que está entrando no salão.
// Saída: Mensagens no terminal indicando qual espectador entrou no salão
// Também atualiza o número de vagas disponíveis.
void entra_salao_publico(int id) {
  printf("Espectador %d está aguardando para entrar no salão\n", id);
  // Aguarda a liberação do semáforo para entrar no salão
  sem_wait(&sem_publico_entrada);
  // Aguarda acesso à variável compartilhada `vagas_publico_utilizadas`
  sem_wait(&sem_vagas_publico_utilizadas);
  vagas_publico_utilizadas++;
  printf("Espectador %d entrou no salão para assistir os testes\n", id);
  // Libera o semáforo para permitir que outros acessem a variável
  sem_post(&sem_vagas_publico_utilizadas);
}

// Função que permite um espectador sair do salão.
// Parâmetro: Id do Espectador que está saindo do salão.
// Saída: Mensagem no terminal informando qual espectador saiu do salão.
// Atualiza quantas vagas tem para o publico da próxima sessão.
void sai_salao(int id) {
  // Aguarda acesso à variável compartilhada `vagas_publico_utilizadas`
  sem_wait(&sem_vagas_publico_utilizadas);
  vagas_publico_utilizadas--;
  printf("Espectador %d saiu do salão\n", id);
  // Libera o semáforo para permitir que outros acessem a variável
  sem_post(&sem_vagas_publico_utilizadas);
}

// Função que faz um espectador assistir aos testes.
// Parâmetro: Id do Espectador que está assistindo os testes.
// Saída: Mensagem no terminal informando qual espectador está assistindo um teste.
// Utiliza um tempo aleatório para simular que está assistindo.
void assiste_teste(int id) {
  // Aguarda até que a avaliação esteja em andamento
  sem_wait(&sem_avaliacao_andamento);
  printf("Espectador %d está assistindo teste\n", id);
  sleep(rand() % 10);
}

// --------------------------- FUNÇÃO PRINCIPAL DA THREAD PUBLICO ---------------------------
void *publico(void *arg) {
  int id = *(int *)arg;

  while (padawans_restantes > 0) {
    entra_salao_publico(id);
    assiste_teste(id);
    sai_salao(id);
  }

  pthread_exit(NULL);
}
