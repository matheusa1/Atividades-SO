#include "interface.h"
#include "time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Função auxiliar para achar o primeiro cluster livre na FAT.
 * Retorna 0xFFFFFFFF se não encontrar.
 */
static uint32_t find_free_cluster(Fat32Image *img) {
  // Quantidade total de clusters
  uint32_t total_clusters =
      (img->boot_sector.BPB_FATSz32 * img->boot_sector.BPB_BytsPerSec) / 4;

  // Percorre a FAT a partir do cluster 2
  for (uint32_t c = 2; c < total_clusters; c++) {
    uint32_t val = img->fat1[c] & 0x0FFFFFFF;
    if (val == 0x00000000) {
      return c;
    }
  }
  return 0xFFFFFFFF;
}

/**
 * Seta o valor do cluster na FAT (ex.: marcar como fim de cadeia, livre, etc.)
 */
static void set_cluster_value(Fat32Image *img, uint32_t cluster,
                              uint32_t value) {
  img->fat1[cluster] = (img->fat1[cluster] & 0xF0000000) | (value & 0x0FFFFFFF);
}

/**
 * Função que grava as entradas '.' e '..' no cluster recém-alocado.
 */
static void init_directory_cluster(Fat32Image *img, uint32_t cluster_new,
                                   uint32_t cluster_parent) {
  uint32_t sector_size = img->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = compute_cluster_size(img);

  // Aloca buffer para limpar o cluster
  uint8_t *buffer = calloc(cluster_size, 1);
  if (!buffer) {
    fprintf(stderr, "Erro de alocação em init_directory_cluster\n");
    return;
  }

  // Monta endereço desse cluster na área de dados:
  uint32_t first_data_sector =
      img->boot_sector.BPB_RsvdSecCnt +
      (img->boot_sector.BPB_NumFATs * img->boot_sector.BPB_FATSz32);
  uint32_t sector =
      first_data_sector + (cluster_new - 2) * img->boot_sector.BPB_SecPerClus;
  uint32_t cluster_offset_bytes = sector * sector_size;

  // Preenche a entrada de "."
  FAT32_DirEntry *dot = (FAT32_DirEntry *)buffer;
  memset(dot->DIR_Name, ' ', 11);
  dot->DIR_Name[0] = '.';
  dot->DIR_Attr = 0x10; // atributo de diretório
  dot->DIR_FstClusHI = (uint16_t)((cluster_new >> 16) & 0xFFFF);
  dot->DIR_FstClusLO = (uint16_t)(cluster_new & 0xFFFF);
  dot->DIR_FileSize = 0;

  // Preenche a entrada de ".."
  FAT32_DirEntry *dotdot = dot + 1;
  memset(dotdot->DIR_Name, ' ', 11);
  dotdot->DIR_Name[0] = '.';
  dotdot->DIR_Name[1] = '.';
  dotdot->DIR_Attr = 0x10; // atributo de diretório
  dotdot->DIR_FstClusHI = (uint16_t)((cluster_parent >> 16) & 0xFFFF);
  dotdot->DIR_FstClusLO = (uint16_t)(cluster_parent & 0xFFFF);
  dotdot->DIR_FileSize = 0;

  // Grava no arquivo
  fseek(img->file, cluster_offset_bytes, SEEK_SET);
  fwrite(buffer, 1, cluster_size, img->file);
  fflush(img->file);

  free(buffer);
}

/**
 * Tenta criar um novo diretório no diretório corrente (current_cluster).
 */
void mkdirCommand(char *command, Fat32Image *image, uint32_t current_cluster) {
  // Pula "mkdir " (5 caracteres) e espaços
  char *dirname = command + 5; // mkdir
  while (*dirname == ' ')
    dirname++;

  if (strlen(dirname) == 0) {
    printf("Uso: mkdir <nome_diretorio>\n");
    return;
  }

  dirname = fileToUpper(dirname);

  // Converte para FAT 8.3
  char fat_name[11];
  for (int i = 0; i < 11; i++) {
    fat_name[i] = ' ';
  }
  string_to_FAT83(dirname, fat_name);

  // Encontra um cluster livre
  uint32_t new_cluster = find_free_cluster(image);
  if (new_cluster == 0xFFFFFFFF) {
    fprintf(stderr, "Não há cluster livre disponível.\n");
    return;
  }

  // Marca esse cluster como EOC (fim de cadeia)
  set_cluster_value(image, new_cluster, 0x0FFFFFFF);
  write_fat(image);

  // Inicializa o cluster com '.' e '..'
  init_directory_cluster(image, new_cluster, current_cluster);

  // Cria a entrada de diretório no "diretório-pai"
  uint32_t cluster = current_cluster;
  uint32_t sector_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = compute_cluster_size(image);
  int criada = 0;

  time_t rawtime = time(NULL);
  struct tm *lt = localtime(&rawtime);

  uint16_t fat_date = dateToFatDate(lt);
  uint16_t fat_time = timeToFatTime(lt);

  while (!criada) {
    // Calcula o primeiro setor de dados
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
    uint32_t sector =
        first_data_sector + (cluster - 2) * image->boot_sector.BPB_SecPerClus;
    uint32_t cluster_offset = sector * sector_size;

    // Lê o cluster na memória
    uint8_t *buffer = malloc(cluster_size);
    if (!buffer) {
      fprintf(stderr, "Erro de alocação de buffer em mkdir\n");
      return;
    }
    fseek(image->file, cluster_offset, SEEK_SET);
    fread(buffer, 1, cluster_size, image->file);

    FAT32_DirEntry *entry = (FAT32_DirEntry *)buffer;
    int entries = cluster_size / sizeof(FAT32_DirEntry);

    for (int i = 0; i < entries; i++) {
      // 0x00 => entrada livre
      // 0xE5 => entrada marcada como excluída
      if (entry[i].DIR_Name[0] == 0x00 ||
          (unsigned char)entry[i].DIR_Name[0] == 0xE5) {
        // Achamos espaço
        memcpy(entry[i].DIR_Name, fat_name, 11);
        entry[i].DIR_Attr = 0x10; // Diretório
        entry[i].DIR_FstClusHI = (uint16_t)((new_cluster >> 16) & 0xFFFF);
        entry[i].DIR_FstClusLO = (uint16_t)(new_cluster & 0xFFFF);
        entry[i].DIR_FileSize = 0;

        // Ajusta a hora e data de criacao e modificacao
        entry[i].DIR_CrtDate = fat_date;
        entry[i].DIR_CrtTime = fat_time;
        entry[i].DIR_WrtDate = fat_date;
        entry[i].DIR_WrtTime = fat_time;

        // Escreve de volta no disco
        fseek(image->file, cluster_offset, SEEK_SET);
        fwrite(buffer, 1, cluster_size, image->file);
        fflush(image->file);

        free(buffer);
        criada = 1;
        printf("Diretório '%s' criado com cluster %u.\n", dirname, new_cluster);
        break;
      }
    }

    if (!criada) {
      // Não achamos espaço nesse cluster; pega o próximo
      uint32_t next_cluster = get_next_cluster(image, cluster);

      // Se fim de cadeia, precisamos alocar novo cluster para expandir o
      // diretório
      if (next_cluster == 0xFFFFFFFF) {
        uint32_t expand_cluster = find_free_cluster(image);
        if (expand_cluster == 0xFFFFFFFF) {
          fprintf(stderr, "Não há cluster livre para expandir o diretório.\n");
          free(buffer);
          return;
        }
        set_cluster_value(image, cluster, expand_cluster);
        set_cluster_value(image, expand_cluster, 0x0FFFFFFF);
        write_fat(image);

        // Zera o conteúdo do cluster recém-adicionado
        memset(buffer, 0, cluster_size);
        fseek(image->file,
              (first_data_sector +
               (expand_cluster - 2) * image->boot_sector.BPB_SecPerClus) *
                  sector_size,
              SEEK_SET);
        fwrite(buffer, 1, cluster_size, image->file);
        fflush(image->file);

        // Avança para ele e continua
        cluster = expand_cluster;
      } else if (next_cluster == cluster) {
        // Evita loop infinito
        fprintf(stderr, "Loop detectado na cadeia de diretórios.\n");
        free(buffer);
        return;
      } else {
        // Vai para o próximo cluster
        cluster = next_cluster;
      }
      free(buffer);
    }
  }
}
