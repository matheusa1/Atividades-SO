#ifndef _LS_INTERFACE_H
#define _LS_INTERFACE_H

#include "../../DirEntry/interface.h"
#include "../../FAT/interface.h"
#include "../../common/interface.h"
#include <stdlib.h>

/*
    Lista o conteúdo de um diretório
    * @param current_cluster: O cluster atual do diretório a ser listado
    * @param image: A imagem do disco FAT32
*/
void list_directory(uint32_t current_cluster, Fat32Image image);

#endif
