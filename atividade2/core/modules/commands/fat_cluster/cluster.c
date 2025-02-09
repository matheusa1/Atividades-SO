#include "interface.h"

// Função para exibir o conteúdo de um cluster no formato de texto
void show_cluster(Fat32Image *image, uint32_t cluster_num) {
  uint32_t fat_size = image->boot_sector.BPB_FATSz32;
  uint32_t reserved_sectors = image->boot_sector.BPB_RsvdSecCnt;
  uint32_t sector_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = compute_cluster_size(image);

  // Offset do início da área de dados
  uint32_t fat_offset = reserved_sectors * sector_size;
  // Tamanho total de cada FAT, em bytes
  uint32_t fat_bytes = fat_size * sector_size;

  // Data offset = inicio da área de dados
  uint32_t data_offset =
      fat_offset + (image->boot_sector.BPB_NumFATs * fat_bytes);
  // Offset em bytes do cluster que vamos ler
  uint32_t cluster_offset = data_offset + (cluster_num - 2) * cluster_size;

  // Move o ponteiro para o início do cluster
  if (fseek(image->file, cluster_offset, SEEK_SET) != 0) {
    printf("Erro: Não foi possível buscar o cluster %d.\n", cluster_num);
    return;
  }

  // Lê o cluster no buffer
  uint8_t *buffer = malloc(cluster_size); // mudamos para cluster_size
  if (!buffer) {
    printf("Erro de alocação de memória.\n");
    return;
  }
  if (fread(buffer, cluster_size, 1, image->file) != 1) {
    printf("Erro: Não foi possível ler o cluster %d.\n", cluster_num);
    free(buffer);
    return;
  }

  // Exibe o conteúdo em formato texto
  for (int i = 0; i < (int)cluster_size; i++) {
    // Exibe caracteres imprimíveis (ASCII 32..126) ou '.' para os
    // não-imprimíveis
    printf("%c", (buffer[i] >= 32 && buffer[i] <= 126) ? buffer[i] : '.');
    if ((i + 1) % 64 == 0)
      printf("\n");
  }
  printf("\n");
  free(buffer);
}