#include "../DirEntry/interface.h"
#include "interface.h"
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>

uint32_t dir_stack[MAX_PATH_DEPTH];
int stack_top = -1;
char currentPath[256] = "/";
uint32_t current_dir;

// Função para obter o próximo cluster
uint32_t get_next_cluster(Fat32Image *img, uint32_t cluster) {
  // Lê quantas entradas de 32 bits há na FAT1
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

  value &= 0x0FFFFFFF;

  // Se >= 0x0FFFFFF8, fim de cadeia
  if (value >= 0x0FFFFFF8) {
    return 0xFFFFFFFF;
  }
  return value;
}

// Função para converter um nome de arquivo para o formato 8.3
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

// Função para calcular o tamanho de um cluster
uint32_t compute_cluster_size(Fat32Image *image) {
  return image->boot_sector.BPB_SecPerClus * image->boot_sector.BPB_BytsPerSec;
}

// Função para encontrar um diretório em um cluster
uint32_t find_directory_cluster(Fat32Image *img, uint32_t start_cluster,
                                const char *dirname) {
  // Percorre o diretório 'start_cluster' procurando por 'dirname'.
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
      if ((unsigned char)entry[i].DIR_Name[0] == 0xE5)
        continue;
      if (entry[i].DIR_Attr & 0x08)
        continue;

      // Verifica se é um diretório
      if ((entry[i].DIR_Attr & 0x10) == 0) {
        // Não é diretório, ignore
        continue;
      }

      // Converte o nome “8.3” para string legível
      char name[13];
      convert_to_83(entry[i].DIR_Name, name);

      // Compara o nome do diretório
      if (strcmp(name, dirname) == 0) {
        // Monta o cluster do subdiretório
        uint32_t hi = entry[i].DIR_FstClusHI;
        uint32_t lo = entry[i].DIR_FstClusLO;
        uint32_t subdir_cluster = (hi << 16) | lo;

        free(buffer);
        return subdir_cluster;
      }
    }

    free(buffer);

    // Caso não encontre o diretório no cluster atual, busca no próximo
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
}

// Função para converter uma string para o formato FAT83
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

// Função para converter uma data para o formato FAT16
uint16_t dateToFatDate(struct tm *lt) {
  int year = lt->tm_year + 1900; // tm_year conta desde 1900
  int month = lt->tm_mon + 1;    // tm_mon vai de 0 a 11
  int day = lt->tm_mday;

  uint16_t fat_date = ((year - 1980) << 9) | (month << 5) | day;

  return fat_date;
}

// Função para converter uma hora para o formato FAT16
uint16_t timeToFatTime(struct tm *lt) {
  int hour = lt->tm_hour;
  int minute = lt->tm_min;
  int second = lt->tm_sec / 2; // segundos divididos por 2

  uint16_t fat_time = (hour << 11) | (minute << 5) | second;

  return fat_time;
}

// Função para atualizar (escrever) a FAT no disco.
// Se BPB_NumFATs == 2, também escreverá na segunda FAT.
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

// Converte um nome de arquivo para maiúsculas.
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

// Procura um arquivo em um cluster específico.
int findFileOnCluster(const char *filename, uint32_t *foundFileCluster,
                      uint32_t *foundFileSize, Fat32Image *image,
                      uint32_t dirCluster) {
  uint32_t sec_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = sec_size * image->boot_sector.BPB_SecPerClus;

  // Vamos percorrer a cadeia de clusters do diretório
  uint32_t cluster = dirCluster;
  int foundEntry = 0;
  uint32_t fileCluster = 0;
  uint32_t fileSize = 0;

  while (!foundEntry && cluster != 0xFFFFFFFF) {
    // Cálculo do primeiro setor do cluster atual
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
    uint32_t sector_num =
        first_data_sector + (cluster - 2) * image->boot_sector.BPB_SecPerClus;
    uint32_t cluster_offset = sector_num * sec_size;

    // Lê o cluster do diretório
    uint8_t *buffer = (uint8_t *)malloc(cluster_size);
    if (!buffer) {
      fprintf(stderr, "Erro de alocação de memória em buscaEntradaArquivo.\n");
      return -1;
    }
    fseek(image->file, cluster_offset, SEEK_SET);
    if (fread(buffer, cluster_size, 1, image->file) != 1) {
      free(buffer);
      fprintf(stderr, "Erro ao ler cluster em buscaEntradaArquivo.\n");
      return -1;
    }

    FAT32_DirEntry *entry = (FAT32_DirEntry *)buffer;
    int entries_per_cluster = cluster_size / sizeof(FAT32_DirEntry);

    for (int i = 0; i < entries_per_cluster; i++) {
      // 0x00 => fim das entradas
      if (entry[i].DIR_Name[0] == 0x00) {
        break;
      }
      // 0xE5 => entrada marcada como livre/apagada
      if ((unsigned char)entry[i].DIR_Name[0] == 0xE5)
        continue;
      // Se for volume label, ignora
      if (entry[i].DIR_Attr & 0x08)
        continue;

      // Converte o nome 8.3 da entrada para string
      char name[13];
      convert_to_83(entry[i].DIR_Name, name);

      // Compara com o filename (supondo que filename já esteja em caixa alta)
      if (strcmp(name, filename) == 0) {
        // Achamos o arquivo
        foundEntry = 1;
        fileCluster = (entry[i].DIR_FstClusHI << 16) | entry[i].DIR_FstClusLO;
        fileSize = entry[i].DIR_FileSize;
        break;
      }
    }

    free(buffer);

    if (!foundEntry) {
      // Pega o próximo cluster do diretório
      uint32_t next_cluster = get_next_cluster(image, cluster);
      if (next_cluster == 0xFFFFFFFF || next_cluster == cluster) {
        // Fim da cadeia ou loop infinito
        break;
      }
      cluster = next_cluster;
    }
  }

  if (foundEntry && fileCluster != 0) {
    *foundFileCluster = fileCluster;
    *foundFileSize = fileSize;
    return 0; // sucesso
  }

  return -1; // não encontrou
}

// Encontra e marca um cluster livre na FAT, devolve o número dele, ou
// 0xFFFFFFFF se não houver
uint32_t allocate_free_cluster(Fat32Image *image) {
  // Lendo algumas informações
  uint32_t total_fat_entries =
      (image->boot_sector.BPB_FATSz32 * image->boot_sector.BPB_BytsPerSec) /
      sizeof(uint32_t);

  // Começa no primeiro cluster livre
  uint32_t start = image->fs_info.FSI_Nxt_Free;
  if (start < 2 || start >= total_fat_entries) {
    start = 2; // Inicializa com o primeiro cluster livre
  }

  // Procura um cluster livre (valor 0x00000000 na FAT)
  for (uint32_t i = start; i < total_fat_entries; i++) {
    if ((image->fat1[i] & 0x0FFFFFFF) == 0x00000000) {
      // cluster livre
      // marca fim-de-cadeia (0x0FFFFFFF)
      image->fat1[i] = 0x0FFFFFFF; // EOC
      // Retorna este cluster
      return i;
    }
  }
  // Não encontrou a partir de start, tenta do 2 até start
  // (pode cobrir o caso em que Nxt_Free > real)
  for (uint32_t i = 2; i < start; i++) {
    if ((image->fat1[i] & 0x0FFFFFFF) == 0x00000000) {
      image->fat1[i] = 0x0FFFFFFF;
      return i;
    }
  }
  // Se chegou aqui, não há clusters livres
  return 0xFFFFFFFF;
}

// Atualiza informações do sistema de arquivos
void update_fsinfo(Fat32Image *image, uint32_t just_allocated) {
  // Decrementa contagem livre se não for 0xFFFFFFFF
  if (image->fs_info.FSI_Free_Count != 0xFFFFFFFF) {
    if (image->fs_info.FSI_Free_Count > 0)
      image->fs_info.FSI_Free_Count--;
  }

  // Atualiza FSI_Nxt_Free
  image->fs_info.FSI_Nxt_Free = just_allocated + 1;

  // Reposiciona e escreve
  fseek(image->file,
        image->boot_sector.BPB_FSInfo * image->boot_sector.BPB_BytsPerSec,
        SEEK_SET);
  fwrite(&(image->fs_info), sizeof(image->fs_info), 1, image->file);
  fflush(image->file);
}

// Encontra uma entrada de diretório
int findDirEntry(const char *filename, FAT32_DirEntry *entry, Fat32Image *image,
                 uint32_t current_cluster) {
  uint32_t cluster = current_cluster;
  uint32_t sector_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = image->boot_sector.BPB_SecPerClus * sector_size;

  char filename_fat[11];
  string_to_FAT83(filename, filename_fat);

  while (cluster != 0xFFFFFFFF) {
    // Calcula o setor inicial do cluster
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
    uint32_t sector =
        first_data_sector + (cluster - 2) * image->boot_sector.BPB_SecPerClus;

    // Lê o cluster
    uint8_t *buffer = malloc(cluster_size);
    if (!buffer)
      return -1;

    fseek(image->file, sector * sector_size, SEEK_SET);
    if (fread(buffer, cluster_size, 1, image->file) != 1) {
      free(buffer);
      return -1;
    }

    // Varre as entradas do diretório
    FAT32_DirEntry *dir_entries = (FAT32_DirEntry *)buffer;
    int entries_per_cluster = cluster_size / sizeof(FAT32_DirEntry);

    for (int i = 0; i < entries_per_cluster; i++) {
      // Fim das entradas
      if (dir_entries[i].DIR_Name[0] == 0x00) {
        free(buffer);
        return -1;
      }

      // Entrada apagada
      if ((unsigned char)dir_entries[i].DIR_Name[0] == 0xE5)
        continue;

      // Compara os nomes
      if (memcmp(dir_entries[i].DIR_Name, filename_fat, 11) == 0) {
        // Encontrou! Copia a entrada e retorna
        memcpy(entry, &dir_entries[i], sizeof(FAT32_DirEntry));
        free(buffer);
        return 0;
      }
    }

    free(buffer);

    // Próximo cluster
    cluster = get_next_cluster(image, cluster);
  }

  return -1; // Não encontrado
}

// Atualiza uma entrada de diretório existente
void updateDirEntry(const char *filename, FAT32_DirEntry *entry,
                    Fat32Image *image, uint32_t current_cluster) {
  uint32_t cluster = current_cluster;
  uint32_t sector_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = image->boot_sector.BPB_SecPerClus * sector_size;

  char filename_fat[11];
  string_to_FAT83(filename, filename_fat);

  while (cluster != 0xFFFFFFFF) {
    // Calcula o setor inicial do cluster
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
    uint32_t sector =
        first_data_sector + (cluster - 2) * image->boot_sector.BPB_SecPerClus;

    // Lê o cluster
    uint8_t *buffer = malloc(cluster_size);
    if (!buffer)
      return;

    fseek(image->file, sector * sector_size, SEEK_SET);
    if (fread(buffer, cluster_size, 1, image->file) != 1) {
      free(buffer);
      return;
    }

    // Varre as entradas do diretório
    FAT32_DirEntry *dir_entries = (FAT32_DirEntry *)buffer;
    int entries_per_cluster = cluster_size / sizeof(FAT32_DirEntry);

    for (int i = 0; i < entries_per_cluster; i++) {
      if (dir_entries[i].DIR_Name[0] == 0x00) {
        free(buffer);
        return;
      }

      if ((unsigned char)dir_entries[i].DIR_Name[0] == 0xE5)
        continue;

      if (memcmp(dir_entries[i].DIR_Name, filename_fat, 11) == 0) {
        // Encontrou! Atualiza a entrada
        memcpy(&dir_entries[i], entry, sizeof(FAT32_DirEntry));

        // Escreve o cluster atualizado de volta no disco
        fseek(image->file, sector * sector_size, SEEK_SET);
        fwrite(buffer, cluster_size, 1, image->file);
        fflush(image->file);

        free(buffer);
        return;
      }
    }

    free(buffer);
    cluster = get_next_cluster(image, cluster);
  }
}
