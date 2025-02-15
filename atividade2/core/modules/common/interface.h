#ifndef _COMMON_INTERFACE_H
#define _COMMON_INTERFACE_H

#include "../DirEntry/interface.h"
#include "../FAT/interface.h"
#include <string.h>
#include <sys/types.h>
#include <time.h>

#define MAX_PATH_DEPTH 256

// Variáveis globais para controle de navegação no sistema de arquivos
extern uint32_t
    dir_stack[MAX_PATH_DEPTH]; // Pilha para armazenar clusters de diretórios
extern int stack_top;          // Topo da pilha de diretórios
extern char currentPath[256];  // Caminho atual no sistema de arquivos
extern uint32_t current_dir;   // Cluster do diretório atual

/**
 * Obtém o próximo cluster na cadeia FAT
 * @param img Imagem do sistema de arquivos FAT32
 * @param cluster Número do cluster atual
 * @return Próximo cluster ou 0xFFFFFFFF se for fim de cadeia
 */
uint32_t get_next_cluster(Fat32Image *img, uint32_t cluster);

/**
 * Converte nome no formato FAT 8.3 para string legível
 * @param src Nome no formato FAT 8.3 (11 bytes)
 * @param dst Buffer para receber o nome convertido
 */
void convert_to_83(const char *src, char *dst);

/**
 * Calcula o tamanho de um cluster em bytes
 * @param image Imagem do sistema de arquivos FAT32
 * @return Tamanho do cluster em bytes
 */
uint32_t compute_cluster_size(Fat32Image *image);

/**
 * Procura um diretório pelo nome em um cluster específico
 * @param img Imagem do sistema de arquivos
 * @param start_cluster Cluster inicial para busca
 * @param dirname Nome do diretório a ser procurado
 * @return Cluster do diretório encontrado ou 0xFFFFFFFF se não encontrado
 */
uint32_t find_directory_cluster(Fat32Image *img, uint32_t start_cluster,
                                const char *dirname);

/**
 * Converte uma string para o formato FAT 8.3
 * @param input String de entrada
 * @param out Buffer de 11 bytes para receber nome no formato FAT
 */
void string_to_FAT83(const char *input, char out[11]);

/**
 * Converte struct tm para data no formato FAT
 * @param lt Ponteiro para struct tm
 * @return Data no formato FAT16
 */
uint16_t dateToFatDate(struct tm *lt);

/**
 * Converte struct tm para hora no formato FAT
 * @param lt Ponteiro para struct tm
 * @return Hora no formato FAT16
 */
uint16_t timeToFatTime(struct tm *lt);

/**
 * Escreve a FAT atualizada no disco
 * @param img Imagem do sistema de arquivos
 */
void write_fat(Fat32Image *img);

/**
 * Converte nome de arquivo para maiúsculas
 * @param filename Nome do arquivo
 * @return Nova string em maiúsculas (deve ser liberada) ou NULL em caso de erro
 */
char *fileToUpper(char *filename);

/**
 * Procura um arquivo em um cluster de diretório
 * @param filename Nome do arquivo
 * @param foundFileCluster Ponteiro para receber cluster do arquivo
 * @param foundFileSize Ponteiro para receber tamanho do arquivo
 * @param image Imagem do sistema de arquivos
 * @param dirCluster Cluster do diretório
 * @return 0 se encontrou, -1 caso contrário
 */
int findFileOnCluster(const char *filename, uint32_t *foundFileCluster,
                      uint32_t *foundFileSize, Fat32Image *image,
                      uint32_t dirCluster);

/**
 * Atualiza informações do sistema de arquivos após alocação
 * @param image Imagem do sistema de arquivos
 * @param just_allocated Cluster recém alocado
 */
void update_fsinfo(Fat32Image *image, uint32_t just_allocated);

/**
 * Aloca um novo cluster livre
 * @param image Imagem do sistema de arquivos
 * @return Número do cluster alocado ou 0xFFFFFFFF se não houver clusters livres
 */
uint32_t allocate_free_cluster(Fat32Image *image);

/**
 * Encontra uma entrada de diretório pelo nome do arquivo
 * @param filename Nome do arquivo
 * @param entry Ponteiro para receber a entrada encontrada
 * @param image Imagem do sistema de arquivos
 * @param current_cluster Cluster atual do diretório
 * @return 0 se encontrou, -1 caso contrário
 */
int findDirEntry(const char *filename, FAT32_DirEntry *entry, Fat32Image *image,
                 uint32_t current_cluster);

/**
 * Atualiza uma entrada de diretório existente
 * @param filename Nome do arquivo
 * @param entry Nova entrada a ser gravada
 * @param image Imagem do sistema de arquivos
 * @param current_cluster Cluster atual do diretório
 */
void updateDirEntry(const char *filename, FAT32_DirEntry *entry,
                    Fat32Image *image, uint32_t current_cluster);

#endif
