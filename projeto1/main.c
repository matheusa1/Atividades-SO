#include "modules/common/globals.h"
#include "modules/padawan/interface.h"
#include "modules/publico/interface.h"
#include "modules/yoda/interface.h"
#include <stdio.h>

int main() {
  // Função que inicia todos os semáforos.
  inicia_semaforos();

  pthread_t yoda_thread;
  pthread_t publico_thread[NUM_PUBLICO];
  pthread_t padawan_thread[NUM_PADAWAN];

  int padawan_ids[NUM_PADAWAN];
  int publico_ids[NUM_PUBLICO];

  pthread_create(&yoda_thread, NULL, yoda, NULL);

  for (int i = 1; i <= NUM_PADAWAN; i++) {
    padawan_ids[i - 1] = i;
    // int numero_aleatorio = rand() % 1000;
    pthread_create(&padawan_thread[i - 1], NULL, padawan,
                   (void *)&padawan_ids[i - 1]);
    // usleep(numero_aleatorio);
  }

  for (int i = 1; i <= NUM_PUBLICO; i++) {
    publico_ids[i - 1] = i;
    pthread_create(&publico_thread[i - 1], NULL, publico,
                   (void *)&publico_ids[i - 1]);
  }

  pthread_join(yoda_thread, NULL);

  for (int i = 0; i < NUM_PADAWAN; i++) {
    pthread_join(padawan_thread[i], NULL);
  }

  for (int i = 0; i < NUM_PUBLICO; i++) {
    pthread_cancel(publico_thread[i]);
  }

  // Função que destrói todos os semáforos.
  destroi_semaforos();

  return 0;
}
