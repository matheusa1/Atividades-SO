#include "../DirEntry/interface.h"
#include "interface.h"
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>

uint32_t dir_stack[MAX_PATH_DEPTH];
int stack_top = -1;
char currentPath[256] = "/";
uint32_t current_dir;

uint32_t get_next_cluster(Fat32Image *img, uint32_t cluster) {
  // Quantas entradas de 32 bits há na FAT1?
  uint32_t maxClusters =
      (img->boot_sector.BPB_FATSz32 * img->boot_sector.BPB_BytsPerSec) /
      sizeof(uint32_t);

  // Se o cluster passado estiver fora do limite, devolve fim de cadeia
  if (cluster >= maxClusters) {
    fprintf(stderr,
            "Aviso: cluster %u fora do limite da tabela FAT (max = %u)\n",
            cluster, maxClusters);
    return 0xFFFFFFFF;
  }

  // Lê o valor da FAT
  uint32_t value = img->fat1[cluster];
  // FAT32 usa apenas 28 bits
  value &= 0x0FFFFFFF;

  // Se >= 0x0FFFFFF8, consideramos fim de cadeia
  if (value >= 0x0FFFFFF8) {
    return 0xFFFFFFFF;
  }
  return value;
}

void convert_to_83(const char *src, char *dst) {
  // Copia o nome (8 caracteres) e a extensão (3 caracteres)
  int i, j = 0;

  // Copia o nome (8 caracteres) removendo espaços
  for (i = 0; i < 8; i++) {
    if (src[i] != ' ') {
      dst[j++] = src[i];
    }
  }

  // Adiciona ponto se houver extensão
  if (src[8] != ' ') {
    dst[j++] = '.';
  }

  // Copia a extensão (3 caracteres) removendo espaços
  for (i = 8; i < 11; i++) {
    if (src[i] != ' ') {
      dst[j++] = src[i];
    }
  }

  // Adiciona o terminador nulo
  dst[j] = '\0';
}

uint32_t compute_cluster_size(Fat32Image *image) {
  return image->boot_sector.BPB_SecPerClus * image->boot_sector.BPB_BytsPerSec;
}

uint32_t find_directory_cluster(Fat32Image *img, uint32_t start_cluster,
                                const char *dirname) {
  // Vamos percorrer o diretório 'start_cluster' procurando por 'dirname'.
  uint32_t cluster = start_cluster;
  uint32_t sector_size = img->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = img->boot_sector.BPB_SecPerClus * sector_size;

  while (1) {
    // Calcula qual é o primeiro setor do cluster “cluster”
    uint32_t first_data_sector =
        img->boot_sector.BPB_RsvdSecCnt +
        (img->boot_sector.BPB_NumFATs * img->boot_sector.BPB_FATSz32);
    uint32_t sector =
        first_data_sector + (cluster - 2) * img->boot_sector.BPB_SecPerClus;

    // Aloca buffer para ler esse cluster
    uint8_t *buffer = malloc(cluster_size);
    if (!buffer) {
      fprintf(stderr, "Erro de alocação\n");
      return 0xFFFFFFFF;
    }

    fseek(img->file, sector * sector_size, SEEK_SET);
    if (fread(buffer, cluster_size, 1, img->file) != 1) {
      free(buffer);
      fprintf(stderr, "Erro ao ler cluster\n");
      return 0xFFFFFFFF;
    }

    // Varre as entradas (cada uma tem sizeof(FAT32_DirEntry))
    FAT32_DirEntry *entry = (FAT32_DirEntry *)buffer;
    int entries_per_cluster = cluster_size / sizeof(FAT32_DirEntry);

    for (int i = 0; i < entries_per_cluster; i++) {
      // 0x00 => fim das entradas do diretório
      if (entry[i].DIR_Name[0] == 0x00) {
        free(buffer);
        return 0xFFFFFFFF;
      }
      // Se estiver marcada como apagada (0xE5) ou volume label, ignora
      if (entry[i].DIR_Name[0] == 0xE5)
        continue;
      if (entry[i].DIR_Attr & 0x08)
        continue;

      // Precisamos que seja um diretório (DIR_Attr & 0x10 != 0)
      if ((entry[i].DIR_Attr & 0x10) == 0) {
        // Não é diretório, ignore
        continue;
      }

      // Converte o nome “8.3” para string legível
      char name[13];
      convert_to_83(entry[i].DIR_Name, name);

      // Comparação simples, case-sensitiva:
      // Se quiser ignorar maiúsculas e minúsculas, faça tolower ou algo
      // similar.
      if (strcmp(name, dirname) == 0) {
        // Achamos o subdiretório. Precisamos montar o cluster dele:
        uint32_t hi = entry[i].DIR_FstClusHI;
        uint32_t lo = entry[i].DIR_FstClusLO;
        uint32_t subdir_cluster = (hi << 16) | lo;

        free(buffer);
        return subdir_cluster;
      }
    }

    free(buffer);

    // Se não achamos nesse cluster, buscamos o próximo na cadeia
    uint32_t next_cluster = get_next_cluster(img, cluster);
    if (next_cluster == 0xFFFFFFFF) {
      // Fim da cadeia
      return 0xFFFFFFFF;
    }
    if (next_cluster == cluster) {
      // Loop detectado
      fprintf(stderr, "Cluster apontando para si mesmo. Abortando.\n");
      return 0xFFFFFFFF;
    }
    cluster = next_cluster;
  }
  // Em teoria, não chega aqui; loop while(1) é encerrado antes.
}

void string_to_FAT83(const char *input, char out[11]) {
  // Inicializa com espaços (caractere 0x20)
  for (int i = 0; i < 11; i++) {
    out[i] = ' ';
  }
  // Cria uma cópia da string em caixa alta
  char temp[256];
  strncpy(temp, input, sizeof(temp) - 1);
  temp[sizeof(temp) - 1] = '\0';
  for (int i = 0; temp[i]; i++) {
    temp[i] = toupper((unsigned char)temp[i]);
  }
  // Procura o ponto para separar nome e extensão, se existir
  char *dot = strchr(temp, '.');
  int name_len = 0, ext_len = 0;
  if (dot) {
    name_len = dot - temp;
    ext_len = strlen(dot + 1);
  } else {
    name_len = strlen(temp);
    ext_len = 0;
  }
  if (name_len > 8)
    name_len = 8;
  if (ext_len > 3)
    ext_len = 3;

  // Copia o nome
  for (int i = 0; i < name_len; i++) {
    out[i] = temp[i];
  }
  // Copia a extensão
  for (int i = 0; i < ext_len; i++) {
    out[8 + i] = dot ? dot[1 + i] : ' ';
  }
}

uint16_t dateToFatDate(struct tm *lt) {
  int year = lt->tm_year + 1900; // tm_year conta desde 1900
  int month = lt->tm_mon + 1;    // tm_mon vai de 0 a 11
  int day = lt->tm_mday;

  uint16_t fat_date = ((year - 1980) << 9) | (month << 5) | day;

  return fat_date;
}

uint16_t timeToFatTime(struct tm *lt) {
  int hour = lt->tm_hour;
  int minute = lt->tm_min;
  int second = lt->tm_sec / 2; // segundos divididos por 2

  uint16_t fat_time = (hour << 11) | (minute << 5) | second;

  return fat_time;
}

/**
 * Atualiza (escreve) a FAT no disco.
 * Se BPB_NumFATs == 2, também escreverá na segunda FAT.
 */
void write_fat(Fat32Image *img) {
  // Calcula onde começa a FAT1
  uint32_t reserved_sectors = img->boot_sector.BPB_RsvdSecCnt;
  uint32_t sector_size = img->boot_sector.BPB_BytsPerSec;
  uint32_t fat_size_bytes = img->boot_sector.BPB_FATSz32 * sector_size;

  // Volta para o início da FAT1 e escreve
  fseek(img->file, reserved_sectors * sector_size, SEEK_SET);
  fwrite(img->fat1, 1, fat_size_bytes, img->file);

  // Se houver segunda FAT (geralmente existe), escreve nela também
  if (img->boot_sector.BPB_NumFATs == 2) {
    // Posição onde começa a FAT2
    fseek(img->file, reserved_sectors * sector_size + fat_size_bytes, SEEK_SET);
    fwrite(img->fat1, 1, fat_size_bytes, img->file);
  }
  fflush(img->file);
}

char *fileToUpper(char *filename) {
  char *temp = malloc(strlen(filename) + 1);
  if (!temp) {
    return NULL;
  }
  strcpy(temp, filename);
  for (int i = 0; temp[i]; i++) {
    temp[i] = toupper((unsigned char)temp[i]);
  }
  return temp;
}
