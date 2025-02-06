#ifndef _COMMON_INTERFACE_H
#define _COMMON_INTERFACE_H
#include "../FAT/interface.h"

#include <string.h>
#include <sys/types.h>

uint32_t get_next_cluster(Fat32Image *img, uint32_t cluster);
void convert_to_83(const char *src, char *dst);
uint32_t compute_cluster_size(Fat32Image *image);
uint32_t find_directory_cluster(Fat32Image *img, uint32_t start_cluster,
                                const char *dirname);
#endif
