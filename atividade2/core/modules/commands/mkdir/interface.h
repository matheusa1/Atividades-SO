#ifndef _MKDIR_INTERFACE_H
#define _MKDIR_INTERFACE_H

#include "../../DirEntry/interface.h"
#include "../../common/interface.h"

/**
 * Cria um novo diretório no cluster atual.
 * Usage: mkdir <nome_diretorio>
 */
void mkdirCommand(char *command, Fat32Image *image, uint32_t current_cluster);

#endif
