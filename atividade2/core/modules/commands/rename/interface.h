#ifndef _RENAME_INTERFACE_H
#define _RENAME_INTERFACE_H

#include "../../DirEntry/interface.h"
#include "../../FAT/interface.h"
#include "../../common/interface.h"

void renameCommand(char *command, Fat32Image *image, uint32_t current_cluster);

#endif
