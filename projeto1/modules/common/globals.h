#ifndef COMMON_INTERFACE_H
#define COMMON_INTERFACE_H

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Declaração dos semáforos
extern sem_t sem_publico_entrada;
extern sem_t sem_publico_entrada;
extern sem_t sem_padawan_entrada;
extern sem_t sem_padawan_pronto;
extern sem_t sem_avaliacao_andamento;
extern sem_t sem_avaliacao_resultado;
extern sem_t sem_aguarda_corte_trança;
extern sem_t sem_padawans_restantes;
extern sem_t sem_padawan_saiu;
extern sem_t sem_vagas_publico_utilizadas;

// // Declaração das variáveis globais
extern int resultado_avaliacao;
extern int padawans_restantes;
extern int entrada_disponivel;
extern int vagas_publico_utilizadas;

// Protótipos das funções
void inicia_semaforos();
void destroi_semaforos();

#endif // COMMON_INTERFACE_H
