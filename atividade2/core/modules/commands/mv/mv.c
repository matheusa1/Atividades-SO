#include "../touch/interface.h"
#include "interface.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Verifica se um path começa com "img/"
 */
int isImagePath(const char *path) { return (strncmp(path, "img/", 4) == 0); }

/*
 * Remove o prefixo "img/" de um path
 */
char *stripImagePrefix(const char *path) {
  char *result = (char *)(isImagePath(path) ? path + 4 : path);
  return result;
}

/*
 * Move arquivo da imagem para o sistema host
 */
static int moveFileImageToHost(const char *src_path_in_image,
                               const char *dst_path, Fat32Image *image,
                               uint32_t current_cluster) {
  // Converte o nome do arquivo para o formato FAT 8.3
  char fat_name[11];
  string_to_FAT83(src_path_in_image, fat_name);

  // Para debug
  printf("Debug: Procurando arquivo com nome FAT: '");
  for (int i = 0; i < 11; i++) {
    printf("%c", fat_name[i]);
  }
  printf("'\n");

  // 1. Localiza o arquivo na imagem
  FAT32_DirEntry entry;
  if (findDirEntry(src_path_in_image, &entry, image, current_cluster) < 0) {
    fprintf(stderr, "Arquivo '%s' não encontrado na imagem.\n",
            src_path_in_image);
    return -1;
  }

  // 2. Obtém o cluster inicial e tamanho do arquivo
  uint32_t fileCluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;
  uint32_t fileSize = entry.DIR_FileSize;

  // 3. Cria o arquivo de destino no sistema host
  FILE *fdst = fopen(dst_path, "wb");
  if (!fdst) {
    fprintf(stderr, "Erro ao criar arquivo local '%s': %s\n", dst_path,
            strerror(errno));
    return -1;
  }

  // 4. Copia o conteúdo
  uint32_t sec_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = sec_size * image->boot_sector.BPB_SecPerClus;
  uint8_t *buffer = malloc(cluster_size);
  if (!buffer) {
    fclose(fdst);
    return -1;
  }

  uint32_t currentCluster = fileCluster;
  uint32_t bytesRemaining = fileSize;

  while (currentCluster != 0xFFFFFFFF && bytesRemaining > 0) {
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
    uint32_t sector = first_data_sector +
                      (currentCluster - 2) * image->boot_sector.BPB_SecPerClus;

    uint32_t bytesToRead =
        (bytesRemaining < cluster_size) ? bytesRemaining : cluster_size;

    fseek(image->file, sector * sec_size, SEEK_SET);
    fread(buffer, 1, bytesToRead, image->file);
    fwrite(buffer, 1, bytesToRead, fdst);

    bytesRemaining -= bytesToRead;
    if (bytesRemaining > 0) {
      currentCluster = get_next_cluster(image, currentCluster);
    }
  }

  fclose(fdst);
  free(buffer);

  // 5. Remove a entrada do diretório e libera os clusters
  uint32_t dir_cluster = current_cluster;
  uint32_t cluster_to_free = fileCluster;

  // Primeiro marca a entrada como excluída
  while (dir_cluster != 0xFFFFFFFF) {
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
    uint32_t sector = first_data_sector +
                      (dir_cluster - 2) * image->boot_sector.BPB_SecPerClus;

    uint8_t *dir_buffer = malloc(cluster_size);
    fseek(image->file, sector * sec_size, SEEK_SET);
    fread(dir_buffer, cluster_size, 1, image->file);

    FAT32_DirEntry *entries = (FAT32_DirEntry *)dir_buffer;
    int entries_per_cluster = cluster_size / sizeof(FAT32_DirEntry);

    for (int i = 0; i < entries_per_cluster; i++) {
      if (memcmp(entries[i].DIR_Name, fat_name, 11) == 0) {
        // Marca a entrada como excluída
        entries[i].DIR_Name[0] = 0xE5;

        // Escreve de volta no disco
        fseek(image->file, sector * sec_size, SEEK_SET);
        fwrite(dir_buffer, cluster_size, 1, image->file);
        fflush(image->file);

        free(dir_buffer);
        goto cleanup_fat;
      }
    }

    free(dir_buffer);
    dir_cluster = get_next_cluster(image, dir_cluster);
  }

cleanup_fat:
  // Agora libera os clusters na FAT
  while (cluster_to_free != 0 && cluster_to_free < 0x0FFFFFF8) {
    uint32_t next = get_next_cluster(image, cluster_to_free);
    image->fat1[cluster_to_free] = 0; // Marca como livre
    cluster_to_free = next;
  }

  // Atualiza a FAT no disco
  write_fat(image);

  return 0;
}

/*
 * Move arquivo do sistema host para a imagem
 */
static int moveFileHostToImage(const char *src_path,
                               const char *dst_path_in_image, Fat32Image *image,
                               uint32_t current_cluster) {
  // 1. Abre o arquivo fonte no sistema host
  FILE *fsrc = fopen(src_path, "rb");
  if (!fsrc) {
    fprintf(stderr, "Erro ao abrir arquivo local '%s': %s\n", src_path,
            strerror(errno));
    return -1;
  }

  // 2. Cria o arquivo na imagem
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "touch %s", dst_path_in_image);
  touchCommand(cmd, image, current_cluster);

  // 3. Localiza a entrada criada
  FAT32_DirEntry entry;
  if (findDirEntry(dst_path_in_image, &entry, image, current_cluster) < 0) {
    fprintf(stderr, "Erro ao localizar arquivo criado na imagem.\n");
    fclose(fsrc);
    return -1;
  }

  // 4. Copia o conteúdo
  uint32_t fileCluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;
  uint32_t sec_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = sec_size * image->boot_sector.BPB_SecPerClus;
  uint32_t currentCluster = fileCluster;
  uint32_t totalBytesWritten = 0;

  uint8_t *buffer = malloc(cluster_size);
  if (!buffer) {
    fclose(fsrc);
    return -1;
  }

  size_t bytesRead;
  while ((bytesRead = fread(buffer, 1, cluster_size, fsrc)) > 0) {
    uint32_t sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32) +
        (currentCluster - 2) * image->boot_sector.BPB_SecPerClus;

    fseek(image->file, sector * sec_size, SEEK_SET);
    fwrite(buffer, 1, bytesRead, image->file);
    totalBytesWritten += bytesRead;

    if (bytesRead == cluster_size) {
      uint32_t newCluster = allocate_free_cluster(image);
      if (newCluster == 0xFFFFFFFF) {
        fprintf(stderr, "Sem espaço na imagem.\n");
        break;
      }
      image->fat1[currentCluster] = newCluster;
      image->fat1[newCluster] = 0x0FFFFFFF;
      write_fat(image);
      update_fsinfo(image, newCluster);
      currentCluster = newCluster;
    }
  }

  // 5. Atualiza o tamanho do arquivo
  entry.DIR_FileSize = totalBytesWritten;
  updateDirEntry(dst_path_in_image, &entry, image, current_cluster);

  free(buffer);
  fclose(fsrc);

  // 6. Remove o arquivo original do sistema host
  if (unlink(src_path) != 0) {
    fprintf(stderr,
            "Aviso: não foi possível remover arquivo original '%s': %s\n",
            src_path, strerror(errno));
  }

  return 0;
}

void mvCommand(char *command, Fat32Image *image, uint32_t current_cluster) {
  // Pula o comando "mv " e espaços
  char *args = command + 2;
  while (*args == ' ')
    args++;

  char *src = strtok(args, " ");
  char *dst = strtok(NULL, " ");

  if (!src || !dst) {
    fprintf(stderr, "Uso: mv <origem> <destino>\n");
    return;
  }

  int srcInImage = isImagePath(src);
  int dstInImage = isImagePath(dst);

  if (srcInImage && !dstInImage) {
    // Movendo da imagem para o sistema host
    char *src_name = stripImagePrefix(src);
    // Converte para maiúsculas
    char *upper_src = fileToUpper(src_name);

    if (moveFileImageToHost(upper_src, dst, image, current_cluster) == 0) {
      printf("Arquivo movido com sucesso da imagem para '%s'\n", dst);
    }
    free(upper_src); // Libera a memória alocada por fileToUpper
  } else if (!srcInImage && dstInImage) {
    // Movendo do sistema host para a imagem
    if (moveFileHostToImage(src, stripImagePrefix(dst), image,
                            current_cluster) == 0) {
      printf("Arquivo movido com sucesso para a imagem como '%s'\n",
             stripImagePrefix(dst));
    }
  } else if (srcInImage && dstInImage) {
    // Movendo dentro da imagem
    const char *src_name = stripImagePrefix(src);
    const char *dst_name = stripImagePrefix(dst);

    FAT32_DirEntry srcEntry;
    if (findDirEntry(src_name, &srcEntry, image, current_cluster) < 0) {
      fprintf(stderr, "Arquivo fonte '%s' não encontrado.\n", src_name);
      return;
    }

    uint32_t dstCluster =
        find_directory_cluster(image, current_cluster, dst_name);
    if (dstCluster == 0xFFFFFFFF) {
      fprintf(stderr, "Diretório destino '%s' não encontrado.\n", dst_name);
      return;
    }

    // Move a entrada do diretório
    // ... (resto do código original para mover dentro da imagem)
  } else {
    // Local → Local: use o mv do sistema
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "/bin/mv '%s' '%s'", src, dst);
    system(cmd);
  }
}
