#ifndef _INFO_INTERFACE_H
#define _INFO_INTERFACE_H
#include "../../common/interface.h"

/*
    Mostra informações sobre a imagem do disco FAT32
    * @param image: A imagem do disco FAT32
*/
void Fat_show_info(Fat32Image *image);

#endif
