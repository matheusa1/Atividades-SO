#include "modules/common/globals.h"
#include "modules/padawan/interface.h"
#include "modules/publico/interface.h"
#include "modules/yoda/interface.h"
#include <stdio.h>

#define NUM_PUBLICO 10
#define NUM_PADAWAN 3

int main() {
  inicia_semaforos();
  padawans_restantes = NUM_PADAWAN;

  pthread_t yoda_thread;
  pthread_t publico_thread[NUM_PUBLICO];
  pthread_t padawan_thread[NUM_PADAWAN];

  pthread_create(&yoda_thread, NULL, yoda, NULL);

  for (int i = 0; i < NUM_PUBLICO; i++) {
    pthread_create(&publico_thread[i], NULL, publico, (void *)&i);
  }

  for (int i = 0; i < NUM_PADAWAN; i++) {
    pthread_create(&padawan_thread[i], NULL, padawan, (void *)&i);
  }

  pthread_join(yoda_thread, NULL);

  for (int i = 0; i < NUM_PADAWAN; i++) {
    pthread_join(padawan_thread[i], NULL);
  }

  for (int i = 0; i < NUM_PUBLICO; i++) {
    pthread_cancel(publico_thread[i]);
  }

  return 0;
}
