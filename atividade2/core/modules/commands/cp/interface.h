#ifndef _CP_INTERFACE_H
#define _CP_INTERFACE_H

#include "../../DirEntry/interface.h"
#include "../../common/interface.h"

/*
 * Implementa o comando cp:
 * Uso: cp <src> <dst>
 * <src>/<dst> podem ser caminhos "img/..." (para arquivos/diretórios dentro da
 * imagem) ou caminhos do SO local.
 */
void cpCommand(char *command, Fat32Image *image, uint32_t current_cluster);

#endif
