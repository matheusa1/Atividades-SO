#ifndef _RM_INTERFACE_H
#define _RM_INTERFACE_H
#include "../../DirEntry/interface.h"
#include "../../FAT/interface.h"
#include "../../common/interface.h"
#include <stdlib.h>

/**
 * Executa o comando rm
 * @param entry_name Nome da entrada a ser removida
 * @param image Imagem do sistema de arquivos FAT32
 * @param current_cluster Cluster atual
 */
void rm_command(char *entry_name, Fat32Image *image, uint32_t current_cluster);

#endif
