#include "../common/globals.h"
#include "interface.h"
#include <semaphore.h>

// --------------------------- FUNÇÕES DE AÇÃO DO YODA ---------------------------

// Função que libera a entrada para Padawans e público.
// Saída: Mensagens na tela informando que Yoda liberou o acesso no salão.
void libera_entrada() {
  padawans_no_salao = 0;

  printf("Yoda liberou a entrada dos Padawans por 5 segundos\n");
  // Libera os semáforos para os Padawans entrarem
  for (int i = 0; i < NR_PADAWAN_PERMITIDO; i++) {
    sem_post(&sem_padawan_entrada);
  }

  int vagas_publico = NR_PUBLICO_PERMITIDO - vagas_publico_utilizadas;

  // Marca a entrada como disponível
  entrada_disponivel = 1;

  // Libera os semáforos para o público entrar
  printf("Yoda liberou a entrada do público, %d vagas\n", vagas_publico);
  for (int i = 0; i < vagas_publico; i++) {
    sem_post(&sem_publico_entrada);
  }
}

// Função que tranca o salão e inicia os testes.
// Saída: Mensagens na tela informando que os testes foram iniciados.
void inicia_testes() {
  // Verifica se tem ao menos um padawan no salão antes de iniciar o teste 
  if(padawans_no_salao <= 0 && padawans_restantes > 0){
       sleep(2);
  }
  // Marca a entrada como indisponível
  entrada_disponivel = 0;
  printf("Yoda iniciou os testes\n");

  for (int i = 0; i < vagas_publico_utilizadas; i++) {
    sem_post(&sem_avaliacao_andamento);
  }

   // Libera os semáforos dos Padawans presentes no salão para iniciar os testes
  for (int i = 0; i < padawans_no_salao; i++) {
    int id_padawan = fila_padawans[i];
    sem_post(&sem_padawans[id_padawan]);
    sem_wait(&sem_padawans_output[id_padawan]);
  }
}

// Função para avaliar o Padawan e dar sua nota.
// Parâmetro: Id do Padawan que está sendo avaliado.
// Saída: Mensagens na tela informando que a avaliação foi feita, e salva o resultado da avaliação em um vetor.
void avalia_padawan(int id) {
  printf("Yoda está avaliando Padawan %d\n", id);

  int resultado = rand() % 100;
  if (resultado >= 60) {
    resultado_padawans[id] = 1;
    padawans_aprovados += 1;
  } else {
    resultado_padawans[id] = 0;
  }

  printf("Yoda avaliou Padawan %d\n", id);
}

// Função para revelar os resultados dos Padawans.
// Parâmetro: Id do Padawan que está sendo avaliado.
// Saída: Retorna o resultado das avaliações.
int anuncia_resultado(int id) {
  printf("Yoda irá anunciar o resultado do Padawan %d\n", id);
  if (resultado_padawans[id] == 1) {
    printf("Padawan %d foi aprovado\n", id);
  } else {
    printf("Padawan %d foi reprovado\n", id);
  }
  // Libera o semáforo para permitir que o Padawan prossig
  sem_post(&sem_padawans[id]);
  sem_wait(&sem_padawans_output[id]);
  return resultado_padawans[id];
}

// Função para cumprimentar os Padawans reprovados.
// Parâmetro: Id do Padawan que foi reprovado.
// Saída: Mensagem no terminal informando que o cumprimento foi feito.
void cumprimenta_Padawan(int id) {
  printf("Yoda está cumprimentando Padawan %d\n", id);
  // Sincroniza com o semáforo do Padawan
  sem_post(&sem_padawans[id]);
  sem_wait(&sem_padawans_output[id]);
}

// Função que corta a trança do Padawan aprovado
// Parâmetro: Id do Padawan aprovado.
// Saída: Mensagem no terminal informando que o corte está sendo realizado.
void corta_tranca(int id) {
  printf("Yoda está cortando a trança do Padawan %d\n", id);
  // Sincroniza com o semáforo do Padawan
  sem_post(&sem_padawans[id]);
  sem_wait(&sem_padawans_output[id]);
}

// --------------------------- FUNÇÃO PRINCIPAL DA THREAD YODA ---------------------------

void *yoda(void *args) {
  
  // Enquanto houver Padawans restantes para avaliar
  while (padawans_restantes > 0) {

    libera_entrada();
    // Coloca o Yoda para dormir por 5 segundos para os padawans entrarem no salão
    sleep(5);

     // Exibe a fila dos Padawans por ordem de chegada
    printf(
        "Fila de Padawans por ordem de chegada para realização dos testes: [");
    for (int i = 0; i < padawans_no_salao; i++) {
      printf("%d, ", fila_padawans[i]);
    }
    printf("\b\b]\n");

    inicia_testes();
    // Avalia cada Padawan presente no salão
    for (int i = 0; i < padawans_no_salao; i++) {
      avalia_padawan(fila_padawans[i]);
      int resultado = anuncia_resultado(fila_padawans[i]);
      if (resultado == 1) {
        corta_tranca(fila_padawans[i]);
      } else {
        cumprimenta_Padawan(fila_padawans[i]);
      }
    }
  }

  // Mensagens finais após todos os Padawans serem avaliados
  // A mensagem será de acordo com o número de Padawans aprovados

  printf("Yoda finalizou o dia\n");

  if (padawans_aprovados == NUM_PADAWAN) {
    printf("Hum... Sentir a Forca, eu posso. Emocionado, eu estou. Orgulho, em "
           "meu coracao, transborda. Padawans, voces sao. Jedi, agora, voces "
           "se tornaram.\n\n");
    printf("Longo e arduo, o caminho foi. Desafios, muitos enfrentaram. Medo, "
           "superaram. Duvidas, venceram. A Forca, em cada um de voces, forte "
           "e. A sabedoria, a coragem, a compaixao, demonstraram.\n\n");
    printf("Lembrar, sempre devem, que ser Jedi, nao e apenas um titulo. Uma "
           "responsabilidade, e. Proteger, servir, e guiar, voces devem. A "
           "paz, a justica, e a harmonia, preservar, voces devem.\n\n");
    printf("Orgulhoso, eu estou, de cada um de voces. O futuro, brilhante e, "
           "com Jedi como voces. A galaxia, em boas maos, esta. A Forca, com "
           "voces, sempre estara.\n\n");
    printf("Lembrem-se, humildes, permanecer. Aprender, sempre continuar. A "
           "Forca, um caminho sem fim, e. Unidos, fortes somos. Divididos, "
           "caimos.\n\n");
    printf("Parabens, jovens Jedi. O caminho, apenas comecou. Que a Forca, com "
           "voces, sempre esteja.\n");
  } else if (padawans_aprovados == 0) {
    printf("Hum... Sentir a Forca, eu posso. Triste, eu estou. Desapontado, "
           "meu coracao, esta. Padawans, voces sao. Jedi, ainda, voces nao se "
           "tornaram.\n\n");
    printf("Difícil, o caminho e. Desafios, muitos enfrentaram. Medo, nao "
           "superaram. Duvidas, nao venceram. A Forca, em cada um de voces, "
           "ainda fraca e. A sabedoria, a coragem, a compaixao, ainda precisam "
           "demonstrar.\n\n");
    printf("Lembrar, sempre devem, que ser Jedi, nao e apenas um titulo. Uma "
           "responsabilidade, e. Proteger, servir, e guiar, voces devem. A "
           "paz, a justica, e a harmonia, preservar, voces devem.\n\n");
    printf("Desapontado, eu estou, mas a esperanca, nao perdi. O futuro, ainda "
           "brilhante pode ser, se esforcar, voces continuarem. A galaxia, em "
           "boas maos, estara, quando prontos, voces estiverem. A Forca, com "
           "voces, sempre estara, se acreditar, voces fizerem.\n\n");
    printf("Lembrem-se, humildes, permanecer. Aprender, sempre continuar. A "
           "Forca, um caminho sem fim, e. Unidos, fortes somos. Divididos, "
           "caimos.\n\n");
    printf("Nao desistir, voces devem. O caminho, dificil e, mas possivel. Que "
           "a Forca, com voces, sempre esteja.\n");
  } else if (padawans_aprovados >= NUM_PADAWAN / 2) {
    printf(
        "Hum... Sentir a Forca, eu posso. Emocionado e orgulhoso, eu estou, "
        "de muitos de voces. Triste e desapontado, tambem estou, por outros. "
        "Padawans, voces sao. Jedi, alguns de voces, agora se tornaram.\n\n");
    printf("Longo e arduo, o caminho foi. Desafios, muitos enfrentaram. Medo, "
           "alguns superaram. Duvidas, alguns venceram. A Forca, em muitos de "
           "voces, forte e. A sabedoria, a coragem, a compaixao, "
           "demonstraram.\n\n");
    printf("Lembrar, sempre devem, que ser Jedi, nao e apenas um titulo. Uma "
           "responsabilidade, e. Proteger, servir, e guiar, voces devem. A "
           "paz, a justica, e a harmonia, preservar, voces devem.\n\n");
    printf("Orgulhoso, eu estou, de cada um que passou. O futuro, brilhante e, "
           "com Jedi como voces. A galaxia, em boas maos, esta. A Forca, com "
           "voces, sempre estara.\n\n");
    printf("Para aqueles que nao passaram, desapontado, eu estou. Mas a "
           "esperanca, nao perdi. O caminho, dificil e, mas possivel. "
           "Aprender, voces devem. Crescer, voces podem. A Forca, com voces, "
           "ainda estara, se acreditar, voces fizerem.\n\n");
    printf("Lembrem-se, humildes, permanecer. Aprender, sempre continuar. A "
           "Forca, um caminho sem fim, e. Unidos, fortes somos. Divididos, "
           "caimos.\n\n");
    printf("Parabens, jovens Jedi, que passaram. E para aqueles que nao "
           "passaram, nao desistir, voces devem. Que a Forca, com todos voces, "
           "sempre esteja.\n");
  } else if (padawans_aprovados < NUM_PADAWAN / 2) {
    printf("Hum... Sentir a Forca, eu posso. Desapontado, eu estou, mas muito "
           "orgulhoso, tambem estou, daqueles que passaram. Padawans, voces "
           "sao. Jedi, alguns de voces, agora se tornaram.\n\n");
    printf("Longo e arduo, o caminho foi. Desafios, muitos enfrentaram. Medo, "
           "alguns superaram. Duvidas, alguns venceram. A Forca, em poucos de "
           "voces, forte e. A sabedoria, a coragem, a compaixao, "
           "demonstraram.\n\n");
    printf("Lembrar, sempre devem, que ser Jedi, nao e apenas um titulo. Uma "
           "responsabilidade, e. Proteger, servir, e guiar, voces devem. A "
           "paz, a justica, e a harmonia, preservar, voces devem.\n\n");
    printf("Orgulhoso, eu estou, de cada um que passou. O futuro, brilhante e, "
           "com Jedi como voces. A galaxia, em boas maos, esta. A Forca, com "
           "voces, sempre estara.\n\n");
    printf("Para aqueles que nao passaram, desapontado, eu estou. Mas a "
           "esperanca, nao perdi. O caminho, dificil e, mas possivel. "
           "Aprender, voces devem. Crescer, voces podem. A Forca, com voces, "
           "ainda estara, se acreditar, voces fizerem.\n\n");
    printf("Lembrem-se, humildes, permanecer. Aprender, sempre continuar. A "
           "Forca, um caminho sem fim, e. Unidos, fortes somos. Divididos, "
           "caimos.\n\n");
    printf("Parabens, jovens Jedi, que passaram. E para aqueles que nao "
           "passaram, nao desistir, voces devem. Que a Forca, com todos voces, "
           "sempre esteja.\n");
  }

  pthread_exit(NULL);
}
