#include "interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Verifica se o diretório está vazio
// Retorna 1 se vazio, 0 se não vazio
static int isDirEmpty(Fat32Image *image, uint32_t dir_cluster) {
  uint32_t cluster = dir_cluster;
  uint32_t sector_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = image->boot_sector.BPB_SecPerClus * sector_size;
  int dot_entries = 0; // Contador para "." e ".."

  while (cluster != 0xFFFFFFFF) {
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
    uint32_t sector =
        first_data_sector + (cluster - 2) * image->boot_sector.BPB_SecPerClus;

    uint8_t *buffer = malloc(cluster_size);
    if (!buffer) {
      fprintf(stderr, "Erro de alocação de memória\n");
      return 0;
    }

    fseek(image->file, sector * sector_size, SEEK_SET);
    if (fread(buffer, cluster_size, 1, image->file) != 1) {
      free(buffer);
      return 0;
    }

    FAT32_DirEntry *entries = (FAT32_DirEntry *)buffer;
    int entries_per_cluster = cluster_size / sizeof(FAT32_DirEntry);

    for (int i = 0; i < entries_per_cluster; i++) {
      // Fim das entradas
      if (entries[i].DIR_Name[0] == 0x00) {
        free(buffer);
        return (dot_entries == 2); // Vazio se só encontrou "." e ".."
      }

      // Pula entradas excluídas
      if ((unsigned char)entries[i].DIR_Name[0] == 0xE5)
        continue;

      // Verifica se é "." ou ".."
      if (entries[i].DIR_Name[0] == '.') {
        if (entries[i].DIR_Name[1] == ' ' || // "."
            (entries[i].DIR_Name[1] == '.' &&
             entries[i].DIR_Name[2] == ' ')) { // ".."
          dot_entries++;
          continue;
        }
      }

      // Se chegou aqui, encontrou uma entrada válida que não é "." ou ".."
      free(buffer);
      return 0; // Diretório não está vazio
    }

    free(buffer);

    cluster = get_next_cluster(image, cluster);
    if (cluster == 0xFFFFFFFF || cluster == dir_cluster)
      break;
  }

  return (dot_entries == 2); // Vazio se só encontrou "." e ".."
}

void rmdirCommand(char *command, Fat32Image *image, uint32_t current_cluster) {
  // Pula "rmdir " e espaços
  char *dirname = command + 5;
  while (*dirname == ' ')
    dirname++;

  if (strlen(dirname) == 0) {
    printf("Uso: rmdir <nome_diretorio>\n");
    return;
  }

  dirname = fileToUpper(dirname);

  // Procura pela entrada do diretório
  FAT32_DirEntry entry;
  if (findDirEntry(dirname, &entry, image, current_cluster) < 0) {
    fprintf(stderr, "Diretório '%s' não encontrado.\n", dirname);
    return;
  }

  // Verifica se é realmente um diretório
  if (!(entry.DIR_Attr & 0x10)) {
    fprintf(stderr, "'%s' não é um diretório.\n", dirname);
    return;
  }

  // Obtém o cluster inicial do diretório
  uint32_t dir_cluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;

  // Verifica se o diretório está vazio
  if (!isDirEmpty(image, dir_cluster)) {
    fprintf(stderr, "Erro: diretório '%s' não está vazio.\n", dirname);
    return;
  }

  // Se chegou aqui, podemos remover o diretório
  // 1. Marca a entrada como excluída
  uint32_t cluster = current_cluster;
  uint32_t sector_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = image->boot_sector.BPB_SecPerClus * sector_size;

  char fat_name[11];
  string_to_FAT83(dirname, fat_name);

  while (cluster != 0xFFFFFFFF) {
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
    uint32_t sector =
        first_data_sector + (cluster - 2) * image->boot_sector.BPB_SecPerClus;

    uint8_t *buffer = malloc(cluster_size);
    if (!buffer) {
      fprintf(stderr, "Erro de alocação de memória\n");
      return;
    }

    fseek(image->file, sector * sector_size, SEEK_SET);
    if (fread(buffer, cluster_size, 1, image->file) != 1) {
      free(buffer);
      return;
    }

    FAT32_DirEntry *entries = (FAT32_DirEntry *)buffer;
    int entries_per_cluster = cluster_size / sizeof(FAT32_DirEntry);

    for (int i = 0; i < entries_per_cluster; i++) {
      if (entries[i].DIR_Name[0] == 0x00) {
        free(buffer);
        goto cleanup;
      }

      if (memcmp(entries[i].DIR_Name, fat_name, 11) == 0) {
        // Marca como excluída
        entries[i].DIR_Name[0] = 0xE5;

        // Escreve de volta no disco
        fseek(image->file, sector * sector_size, SEEK_SET);
        fwrite(buffer, cluster_size, 1, image->file);
        fflush(image->file);

        free(buffer);
        goto cleanup;
      }
    }

    free(buffer);
    cluster = get_next_cluster(image, cluster);
  }

cleanup:
  // 2. Libera os clusters na FAT
  cluster = dir_cluster;
  while (cluster != 0xFFFFFFFF && cluster < 0x0FFFFFF8) {
    uint32_t next = get_next_cluster(image, cluster);
    image->fat1[cluster] = 0; // Marca como livre
    cluster = next;
  }

  // 3. Atualiza a FAT no disco
  write_fat(image);

  // 4. Atualiza o FSInfo
  image->fs_info.FSI_Free_Count++;
  update_fsinfo(image, dir_cluster);

  printf("Diretório '%s' removido com sucesso.\n", dirname);
}
