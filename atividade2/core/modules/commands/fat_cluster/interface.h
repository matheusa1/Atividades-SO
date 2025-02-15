#ifndef _CLUSTER_INTERFACE_H
#define _CLUSTER_INTERFACE_H
#include "../../common/interface.h"
#include <stdlib.h>

/*
    Mostra informações sobre um cluster do disco FAT32
    * @param image: A imagem do disco FAT32
    * @param cluster_num: O número do cluster a ser mostrado
*/
void show_cluster(Fat32Image *image, uint32_t cluster_num);

#endif
