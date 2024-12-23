#ifndef COMMON_INTERFACE_H
#define COMMON_INTERFACE_H

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_PUBLICO 10
#define NUM_PADAWAN 7
#define NR_PUBLICO_PERMITIDO 5
#define NR_PADAWAN_PERMITIDO NUM_PADAWAN

// Declaração dos semáforos
extern sem_t sem_publico_entrada;
extern sem_t sem_padawan_entrada;
extern sem_t sem_vagas_publico_utilizadas;
extern sem_t sem_vagas_padawans_utilizadas;
extern sem_t sem_cumprimento_mestres;
extern sem_t sem_avaliacao_andamento;
extern sem_t sem_padawans_restantes;

// // Declaração das variáveis globais
extern int entrada_disponivel;
extern int vagas_publico_utilizadas;
extern int padawans_no_salao;
extern int fila_padawans[NUM_PADAWAN];
extern int resultado_padawans[NUM_PADAWAN];
extern sem_t sem_padawans[NUM_PADAWAN];
extern sem_t sem_padawans_output[NUM_PADAWAN];
extern int padawans_aprovados;
extern int padawans_restantes;

// Protótipos das funções
void inicia_semaforos();
void destroi_semaforos();
void insere_padawan_na_fila(int id);
void remove_padawan_da_fila();

#endif // COMMON_INTERFACE_H
