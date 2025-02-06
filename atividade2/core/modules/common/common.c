#include "../DirEntry/interface.h"
#include "interface.h"
#include <stdlib.h>
#include <sys/types.h>

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

      printf("Dir: %s, buscado: %s\n", name, dirname);

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
