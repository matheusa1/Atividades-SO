#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interface.h"

// Função auxiliar para imprimir as flags de atributo
static void print_attr_flags(uint8_t attr) {
  printf("Atributos: ");
  if (attr & 0x01)
    printf("READ_ONLY ");
  if (attr & 0x02)
    printf("HIDDEN ");
  if (attr & 0x04)
    printf("SYSTEM ");
  if (attr & 0x08)
    printf("VOLUME_LABEL ");
  if (attr & 0x10)
    printf("DIRECTORY ");
  if (attr & 0x20)
    printf("ARCHIVE ");
  printf("\n");
}

void attrCommand(char *command, Fat32Image *image, uint32_t current_cluster) {
  // Espera-se que o comando seja: "attr <nome_arquivo>"
  char filename[13];
  // pula o comando "attr" (5 caracteres) e os espaços posteriores
  char *arg = command + 4;
  while (*arg == ' ')
    arg++;
  if (strlen(arg) == 0) {
    fprintf(stderr, "Uso: attr <nome_arquivo>\n");
    return;
  }
  // Copia o nome do arquivo (até 12 caracteres para nome 8.3)
  strncpy(filename, arg, 12);
  filename[12] = '\0';

  // Procura pela entrada com o mesmo nome no diretório corrente
  uint32_t cluster = current_cluster;
  uint32_t sector_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = image->boot_sector.BPB_SecPerClus * sector_size;
  int encontrado = 0;

  do {
    // Calcula o primeiro setor da área de dados
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
    uint32_t sector =
        first_data_sector + (cluster - 2) * image->boot_sector.BPB_SecPerClus;

    uint8_t *buffer = malloc(cluster_size);
    if (!buffer) {
      fprintf(stderr, "Erro de alocação\n");
      return;
    }

    fseek(image->file, sector * sector_size, SEEK_SET);
    if (fread(buffer, cluster_size, 1, image->file) != 1) {
      free(buffer);
      fprintf(stderr, "Erro na leitura do cluster\n");
      return;
    }

    FAT32_DirEntry *entry = (FAT32_DirEntry *)buffer;
    int entries = cluster_size / sizeof(FAT32_DirEntry);

    for (int i = 0; i < entries; i++) {
      // 0x00: fim das entradas
      if (entry[i].DIR_Name[0] == 0x00) {
        free(buffer);
        goto fim;
      }
      // 0xE5: entrada apagada
      if (entry[i].DIR_Name[0] == 0xE5)
        continue;
      // Ignora entradas volume label (bit 0x08)
      if (entry[i].DIR_Attr & 0x08)
        continue;

      // Converte o nome 8.3 para string legível
      char entryName[13];
      convert_to_83(entry[i].DIR_Name, entryName);

      // Se o nome bater, mostra as informações de atributo
      if (strcmp(entryName, filename) == 0) {
        printf("Arquivo: %s\n", entryName);
        printf("Tamanho: %u bytes\n", entry[i].DIR_FileSize);
        printf("Atributo bruto: 0x%X\n", entry[i].DIR_Attr);
        print_attr_flags(entry[i].DIR_Attr);
        encontrado = 1;
        break;
      }
    }
    free(buffer);
    if (encontrado)
      break;

    // Busca próximo cluster do diretório, se houver
    uint32_t next_cluster = get_next_cluster(image, cluster);
    if (next_cluster == 0xFFFFFFFF || next_cluster == cluster)
      break;
    cluster = next_cluster;
  } while (1);

fim:
  if (!encontrado) {
    fprintf(stderr, "Arquivo '%s' não encontrado.\n", filename);
  }
}
