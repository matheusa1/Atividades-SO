#ifndef _RENAME_INTERFACE_H
#define _RENAME_INTERFACE_H

#include "../../DirEntry/interface.h"
#include "../../FAT/interface.h"
#include "../../common/interface.h"

/**
 * Executa o comando rename
 * @param command comando executado
 * @param image imagem do disco FAT32
 * @param current_cluster cluster atual
 */
void renameCommand(char *command, Fat32Image *image, uint32_t current_cluster);

#endif
