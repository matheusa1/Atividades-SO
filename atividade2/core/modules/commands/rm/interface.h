#ifndef _RM_INTERFACE_H
#define _RM_INTERFACE_H
#include "../../common/interface.h"
#include "../../FAT/interface.h"
#include "../../DirEntry/interface.h"
#include <stdlib.h>

void rm_command(char *entry_name, Fat32Image *image, uint32_t current_cluster);


#endif