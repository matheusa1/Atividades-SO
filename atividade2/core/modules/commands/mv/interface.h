#ifndef _MV_INTERFACE_H
#define _MV_INTERFACE_H

#include "../../DirEntry/interface.h"
#include "../../common/interface.h"

/*
 * Implementa o comando mv com as seguintes possibilidades:
 * - mv arquivo.txt img/ARQUIVO.TXT  (move do host para a imagem)
 * - mv img/ARQUIVO.TXT arquivo.txt  (move da imagem para o host)
 * - mv img/ARQUIVO.TXT img/DIR      (move dentro da imagem)
 * - mv arquivo1.txt arquivo2.txt    (move no sistema host)
 */
void mvCommand(char *command, Fat32Image *image, uint32_t current_cluster);

#endif
