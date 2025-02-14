#include "interface.h"

/* Função para converter data/hora em formato FAT (16 bits cada) para string.
   Formato resultante: "YYYY-MM-DD HH:MM:SS".
   - fatDate = [ano-1980:7 | mês:4 | dia:5]
   - fatTime = [hora:5 | min:6 | seg/2:5]
*/
static void fat_datetime_to_string(uint16_t fatDate, uint16_t fatTime,
                                   char *outstr, size_t outsize) {
  // Extrai campos de data
  uint16_t day = fatDate & 0x1F;                  // bits [0..4]
  uint16_t month = (fatDate >> 5) & 0x0F;         // bits [5..8]
  uint16_t year = ((fatDate >> 9) & 0x7F) + 1980; // bits [9..15], desde 1980

  // Extrai campos de hora
  uint16_t sec2 = fatTime & 0x1F;          // bits [0..4] (segundos / 2)
  uint16_t minute = (fatTime >> 5) & 0x3F; // bits [5..10]
  uint16_t hour = (fatTime >> 11) & 0x1F;  // bits [11..15]

  // Calcula segundos reais
  uint16_t second = sec2 * 2;

  // Monta string no formato desejado, ex: "2023-10-05 14:32:04"
  snprintf(outstr, outsize, "%04u-%02u-%02u %02u:%02u:%02u", year, month, day,
           hour, minute, second);
}

void list_directory(uint32_t current_cluster, Fat32Image image) {
  uint32_t cluster = current_cluster;
  uint32_t sector_size = image.boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = compute_cluster_size(&image);

  do {
    uint32_t first_data_sector =
        image.boot_sector.BPB_RsvdSecCnt +
        (image.boot_sector.BPB_NumFATs * image.boot_sector.BPB_FATSz32);
    uint32_t sector =
        first_data_sector + (cluster - 2) * image.boot_sector.BPB_SecPerClus;

    uint8_t *buffer = malloc(cluster_size);
    if (!buffer) {
      fprintf(stderr, "Erro de alocação\n");
      return;
    }

    fseek(image.file, sector * sector_size, SEEK_SET);
    if (fread(buffer, cluster_size, 1, image.file) != 1) {
      free(buffer);
      break;
    }

    FAT32_DirEntry *entry = (FAT32_DirEntry *)buffer;
    for (int i = 0; i < (int)(cluster_size / sizeof(FAT32_DirEntry)); i++) {
      // 0x00 => fim das entradas
      if (entry[i].DIR_Name[0] == 0x00) {
        free(buffer);
        return;
      }

      // 0xE5 => entrada apagada, pular
      if ((unsigned char)entry[i].DIR_Name[0] == 0xE5) {
        continue;
      }

      // 0x08 => volume label, pular
      if (entry[i].DIR_Attr & 0x08)
        continue;

      // Se chegou aqui, é uma entrada válida
      char name[13];
      convert_to_83(entry[i].DIR_Name, name);

      char datetime_created[20];
      fat_datetime_to_string(entry[i].DIR_CrtDate, entry[i].DIR_CrtTime,
                             datetime_created, sizeof(datetime_created));

      char datetime_modified[20];
      fat_datetime_to_string(entry[i].DIR_WrtDate, entry[i].DIR_WrtTime,
                             datetime_modified, sizeof(datetime_modified));

      printf("%10s %10u %13s  (Criado em %s) (Atualizado em %s)\n",
             (entry[i].DIR_Attr & 0x10) ? "[DIR]" : "[FILE]",
             entry[i].DIR_FileSize, name, datetime_created, datetime_modified);
    }

    free(buffer);

    uint32_t next_cluster = get_next_cluster(&image, cluster);
    if (next_cluster == cluster) {
      fprintf(stderr,
              "Alerta: cluster apontando para si mesmo (%u). Encerrando.\n",
              cluster);
      break;
    }
    if (next_cluster == 0xFFFFFFFF) {
      break;
    }
    cluster = next_cluster;
  } while (1);
}
