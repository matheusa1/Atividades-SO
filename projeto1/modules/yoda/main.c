#include "../common/globals.h"
#include "interface.h"
#include <semaphore.h>

#define NR_PUBLICO_PERMITIDO 5

int padawans_aprovados = 0;

void inicia_testes() {
  sem_wait(&sem_padawan_pronto);
  printf("Yoda está iniciando os testes\n");
  entrada_disponivel = 0;

  for (int i = 0; i < NR_PUBLICO_PERMITIDO + 1; i++) {
    sem_post(&sem_avaliacao_andamento);
  }
}

void libera_entrada() {
  printf("Yoda liberou a entrada do Padawan\n");
  sem_post(&sem_padawan_entrada);

  int vagas_publico = NR_PUBLICO_PERMITIDO - vagas_publico_utilizadas;

  entrada_disponivel = 1;
  printf("Yoda liberou a entrada do público, %d vagas\n", vagas_publico);
  for (int i = 0; i < vagas_publico; i++) {
    sem_post(&sem_publico_entrada);
  }
}

void anuncia_resultado() {
  sem_wait(&sem_padawan_pronto);
  printf("Yoda está avaliando o Padawan\n");
  sleep(1);

  srand(rand());

  int numero = rand() % 100;
  printf("Yoda avaliou o Padawan com nota %d/100\n", numero);

  if (numero >= 60) {
    resultado_avaliacao = 1;
    printf("Padawn aprovado 👍\n");
  } else {
    resultado_avaliacao = 2;
    printf("Padawn reprovado 👎\n");
  }

  sem_post(&sem_avaliacao_resultado);
}

void aguarda_cumprimento() {
  sem_wait(&sem_padawan_pronto);
  printf("Yoda foi cumprimentado pelo Padawan\n");
}

void corta_tranca() {
  sem_wait(&sem_aguarda_corte_trança);
  printf("Yoda está cortando a trança do Padawan\n");
  sleep(1);
  printf("Yoda terminou de cortar a trança do Padawan\n");
  sem_post(&sem_padawan_pronto);
}

void finaliza_teste() {
  sem_wait(&sem_padawan_saiu);
  printf("Yoda finalizou o teste\n");
}

void *yoda(void *args) {
  int numero_de_padawans = padawans_restantes;

  while (padawans_restantes > 0) {
    libera_entrada();
    sleep(2);
    inicia_testes();
    anuncia_resultado();
    if (resultado_avaliacao == 2) {
      aguarda_cumprimento();
      finaliza_teste();
      padawans_restantes--;
      continue;
    } else {
      padawans_aprovados++;
      corta_tranca();
      finaliza_teste();
    }
    padawans_restantes--;
  }

  printf("Yoda finalizou o dia\n");

  if (padawans_aprovados == numero_de_padawans) {
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
  } else if (padawans_aprovados >= numero_de_padawans / 2) {
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
  } else if (padawans_aprovados < numero_de_padawans / 2) {
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
