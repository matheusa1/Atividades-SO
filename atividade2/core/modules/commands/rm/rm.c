#include "interface.h"
#define DIR_ENTRY_UNUSED 0xE5

// Libera os clusters ocupados por um arquivo no FAT
void free_cluster(uint32_t *fat, uint32_t cluster, FSInfoStruct *fsInfo) {
  uint32_t cluster_atual = cluster;
  uint32_t proximo_cluster;

  while (cluster_atual != 0xFFFFFFF && cluster_atual < 0xFFFFFF8) {
    proximo_cluster = fat[cluster_atual];
    fat[cluster_atual] = 0x00;
    fsInfo->FSI_Free_Count++;
    cluster_atual = proximo_cluster;
  }
}

// Remove uma entrada de diretório
int rm_arq_entry(FAT32_DirEntry *entry, FILE *file, uint32_t *fat,
                 FAT32_BootSector boot_sector, FSInfoStruct *fsi_info) {
  uint32_t cluster = (entry->DIR_FstClusHI << 16) | entry->DIR_FstClusLO;
  free_cluster(fat, cluster, fsi_info);
  entry->DIR_Name[0] = DIR_ENTRY_UNUSED;
  return 1;
}

void rm_command(char *entry_name, Fat32Image *image, uint32_t current_cluster) {
  // Pula o comando "rm" (2 caracteres) e os espaços seguintes
  char *filename = entry_name + 2;
  while (*filename == ' ')
    filename++;

  // Converte o nome do arquivo para o formato FAT 8.3
  char filename_fat[11];
  string_to_FAT83(filename, filename_fat);

  // Variáveis para percorrer os clusters do diretório
  uint32_t cluster = current_cluster;
  uint32_t sector_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = image->boot_sector.BPB_SecPerClus * sector_size;
  int encontrado = 0;

  do {
    // Calcula o primeiro setor de dados
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);

    // Calcula o setor inicial deste cluster
    uint32_t sector =
        first_data_sector + (cluster - 2) * image->boot_sector.BPB_SecPerClus;

    // Calcula o offset (em bytes) deste cluster
    uint32_t cluster_offset = sector * sector_size;

    // Aloca um buffer para o cluster
    uint8_t *buffer = malloc(cluster_size);
    if (!buffer) {
      fprintf(stderr, "Erro de alocação\n");
      return;
    }

    // Lê o cluster do arquivo
    fseek(image->file, cluster_offset, SEEK_SET);
    if (fread(buffer, cluster_size, 1, image->file) != 1) {
      free(buffer);
      fprintf(stderr, "Erro na leitura do cluster\n");
      return;
    }

    // Varre as entradas do cluster
    FAT32_DirEntry *entry = (FAT32_DirEntry *)buffer;
    int entries = cluster_size / sizeof(FAT32_DirEntry);
    for (int i = 0; i < entries; i++) {
      // Se o primeiro byte for 0x00, fim das entradas
      if (entry[i].DIR_Name[0] == 0x00) {
        free(buffer);
        goto fim;
      }

      // Se a entrada estiver apagada ou for volume label, ignora
      if ((unsigned char)entry[i].DIR_Name[0] == 0xE5)
        continue;
      if (entry[i].DIR_Attr & 0x08)
        continue;

      // Compara o nome da entrada com o nome do arquivo buscado
      if (strncmp(entry[i].DIR_Name, filename_fat, 11) == 0) {
        // Verifica se é um arquivo (atributo 0x20)
        if ((entry[i].DIR_Attr & 0x20) == 0) {
          printf("Erro: '%s' não é um arquivo.\n", filename);
          free(buffer);
          return;
        }

        // Marca a entrada como apagada (0xE5)
        entry[i].DIR_Name[0] = 0xE5;

        // Escreve a entrada modificada de volta no arquivo
        long entry_file_offset = cluster_offset + i * sizeof(FAT32_DirEntry);
        fseek(image->file, entry_file_offset, SEEK_SET);
        if (fwrite(&entry[i], sizeof(FAT32_DirEntry), 1, image->file) != 1) {
          fprintf(stderr, "Erro ao escrever a entrada modificada.\n");
        } else {
          fflush(image->file);
          printf("Arquivo '%s' removido com sucesso.\n", filename);
        }
        encontrado = 1;
        break;
      }
    }
    free(buffer);
    if (encontrado)
      break;

    // Se não encontrou, busca o próximo cluster do diretório
    uint32_t next_cluster = get_next_cluster(image, cluster);
    if (next_cluster == 0xFFFFFFFF || next_cluster == cluster)
      break;
    cluster = next_cluster;
  } while (1);

fim:
  if (!encontrado) {
    printf("Erro: Arquivo '%s' não encontrado.\n", filename);
  }
}
