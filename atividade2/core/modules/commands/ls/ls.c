#include "interface.h"

void list_directory(uint32_t current_cluster, Fat32Image image) {
  uint32_t cluster = current_cluster;
  uint32_t sector_size = image.boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = compute_cluster_size(&image);

  do {
    // Calcular setor do cluster atual
    uint32_t first_data_sector =
        image.boot_sector.BPB_RsvdSecCnt +
        (image.boot_sector.BPB_NumFATs * image.boot_sector.BPB_FATSz32);
    uint32_t sector =
        first_data_sector + (cluster - 2) * image.boot_sector.BPB_SecPerClus;

    // Ler o cluster
    uint8_t *buffer = malloc(cluster_size);
    if (!buffer) {
      fprintf(stderr, "Erro de alocação\n");
      return;
    }

    fseek(image.file, sector * sector_size, SEEK_SET);
    if (fread(buffer, cluster_size, 1, image.file) != 1) {
      free(buffer);
      break; // Erro na leitura
    }

    // Processar entradas
    FAT32_DirEntry *entry = (FAT32_DirEntry *)buffer;
    for (int i = 0; i < (int)(cluster_size / sizeof(FAT32_DirEntry)); i++) {
      // 0x00 => fim das entradas
      if (entry[i].DIR_Name[0] == 0x00)
        break;

      // 0xE5 => entrada apagada, pular
      if (entry[i].DIR_Name[0] == 0xE5)
        continue;

      // 0x08 pode ser "volume label" etc. pular
      if (entry[i].DIR_Attr & 0x08)
        continue;

      // Converter nome para 8.3 legível
      char name[13];
      convert_to_83(entry[i].DIR_Name, name);

      printf("%s %d %s\n", (entry[i].DIR_Attr & 0x10) ? "[DIR]" : "[FILE]",
             entry[i].DIR_FileSize, name);
    }

    free(buffer);

    // Obter próximo cluster
    uint32_t next_cluster = get_next_cluster(&image, cluster);

    if (next_cluster == cluster) {
      fprintf(stderr,
              "Alerta: cluster apontando para si mesmo (%u). Encerrando.\n",
              cluster);
      break;
    }

    if (next_cluster == 0xFFFFFFFF) {
      // Fim da cadeia
      break;
    }
    cluster = next_cluster;

  } while (1);
}
