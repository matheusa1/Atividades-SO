#ifndef _ATTR_INTERFACE_H
#define _ATTR_INTERFACE_H

#include "../../DirEntry/interface.h"
#include "../../FAT/interface.h"
#include "../../common/interface.h"

/*
    Função que implementa o comando attr.
    * @param command: comando a ser executado.
    * @param image: imagem do disco.
    * @param current_cluster: cluster atual.
*/
void attrCommand(char *command, Fat32Image *image, uint32_t current_cluster);

#endif
