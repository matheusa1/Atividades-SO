#ifndef _CD_INTERFACE_H
#define _CD_INTERFACE_H
#include "../../common/interface.h"

void cdCommand(char *command, Fat32Image *image, char *currentPath);

#endif
