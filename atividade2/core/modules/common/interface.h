#ifndef _COMMON_INTERFACE_H
#define _COMMON_INTERFACE_H
#include "../DirEntry/interface.h"
#include "../FAT/interface.h"

#include <string.h>
#include <sys/types.h>
#include <time.h>

#define MAX_PATH_DEPTH 256
extern uint32_t dir_stack[MAX_PATH_DEPTH];
extern int stack_top;
extern char currentPath[256];
extern uint32_t current_dir;

uint32_t get_next_cluster(Fat32Image *img, uint32_t cluster);
void convert_to_83(const char *src, char *dst);
uint32_t compute_cluster_size(Fat32Image *image);
uint32_t find_directory_cluster(Fat32Image *img, uint32_t start_cluster,
                                const char *dirname);
void string_to_FAT83(const char *input, char out[11]);
uint16_t dateToFatDate(struct tm *lt);
uint16_t timeToFatTime(struct tm *lt);
void write_fat(Fat32Image *img);
char *fileToUpper(char *filename);
int findFileOnCluster(const char *filename, uint32_t *foundFileCluster,
                      uint32_t *foundFileSize, Fat32Image *image,
                      uint32_t dirCluster);
void update_fsinfo(Fat32Image *image, uint32_t just_allocated);
uint32_t allocate_free_cluster(Fat32Image *image);
// Encontra e retorna a entrada de diretório de um arquivo
int findDirEntry(const char *filename, FAT32_DirEntry *entry, Fat32Image *image,
                 uint32_t current_cluster);

// Atualiza uma entrada de diretório existente
void updateDirEntry(const char *filename, FAT32_DirEntry *entry,
                    Fat32Image *image, uint32_t current_cluster);
#endif
