#ifndef _LS_INTERFACE_H
#define _LS_INTERFACE_H

#include "../../DirEntry/interface.h"
#include "../../FAT/interface.h"
#include "../../common/interface.h"
#include <stdlib.h>

void list_directory(uint32_t current_cluster, Fat32Image image);

#endif
