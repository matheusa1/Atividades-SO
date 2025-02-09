#include "interface.h"

// Função para exibir informações básicas da FAT32
void Fat_show_info(Fat32Image *image) {
  printf("OEM Name: %s\n", image->boot_sector.BS_OEMName);
  printf("Bytes per sector: %d\n", image->boot_sector.BPB_BytsPerSec);
  printf("Sectors per cluster: %d\n", image->boot_sector.BPB_SecPerClus);
  printf("Reserved sectors: %d\n", image->boot_sector.BPB_RsvdSecCnt);
  printf("Number of FATs: %d\n", image->boot_sector.BPB_NumFATs);
  printf("Sectors per FAT: %d\n", image->boot_sector.BPB_SecPerTrk);
  printf("Root cluster: %d\n", image->boot_sector.BPB_RootClus);
}