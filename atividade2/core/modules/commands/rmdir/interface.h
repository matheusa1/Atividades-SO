#ifndef _RMDIR_INTERFACE_H
#define _RMDIR_INTERFACE_H

#include "../../DirEntry/interface.h"
#include "../../common/interface.h"

/**
 * Executa o comando rmdir
 * @param command Comando a ser executado
 * @param image Imagem do sistema de arquivos FAT32
 * @param current_cluster Cluster atual
 */
void rmdirCommand(char *command, Fat32Image *image, uint32_t current_cluster);

#endif
