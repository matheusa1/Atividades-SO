#ifndef _CD_INTERFACE_H
#define _CD_INTERFACE_H
#include "../../common/interface.h"

/*
    Move pelos diretórios
    * @param command - Comando a ser executado
    * @param image - Imagem do disco FAT32
    * @param currentPath - Caminho atual
*/
void cdCommand(char *command, Fat32Image *image, char *currentPath);

#endif
