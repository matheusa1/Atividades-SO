#ifndef _TOUCH_INTERFACE_H
#define _TOUCH_INTERFACE_H

#include "../../common/interface.h"
#include "../../DirEntry/interface.h"

void touchCommand(char* command, Fat32Image* image, uint32_t current_cluster);

#endif