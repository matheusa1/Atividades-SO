#include "../touch/interface.h"
#include "interface.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * Função para verificar se um dado path começa com "img/".
 * Retorna 1 (true) se sim, 0 se não.
 */
static int isImagePath(const char *path) {
  return (strncmp(path, "img/", 4) == 0);
}

/*
 * Remove o prefixo "img/" de um path que esteja dentro da imagem,
 * retornando a string interna. Ex: "img/MEUARQ.TXT" => "MEUARQ.TXT".
 */
static const char *stripImagePrefix(const char *path) {
  if (isImagePath(path)) {
    return path + 4;
  }
  return path;
}

/*
 * Função auxiliar que copia um arquivo local do SO hospedeiro para dentro da
 * imagem
 */
static int copyFileHostToImage(const char *src_path,
                               const char *dst_path_in_image, Fat32Image *image,
                               uint32_t current_cluster) {
  FILE *fsrc = fopen(src_path, "rb");
  if (!fsrc) {
    fprintf(stderr, "Erro ao abrir arquivo local '%s': %s\n", src_path,
            strerror(errno));
    return -1;
  }

  // Cria o arquivo de destino na imagem usando touch
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "touch %s", dst_path_in_image);
  touchCommand(cmd, image, current_cluster);

  // Localiza a entrada do arquivo recém-criado
  FAT32_DirEntry targetEntry;
  if (findDirEntry(dst_path_in_image, &targetEntry, image, current_cluster) <
      0) {
    fprintf(stderr, "Erro ao localizar arquivo criado na imagem.\n");
    fclose(fsrc);
    return -1;
  }

  uint32_t fileCluster =
      (targetEntry.DIR_FstClusHI << 16) | targetEntry.DIR_FstClusLO;
  uint32_t sec_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = sec_size * image->boot_sector.BPB_SecPerClus;

  uint32_t currentFileCluster = fileCluster;
  uint32_t totalBytesWritten = 0;

  uint8_t *tempBuf = malloc(cluster_size);
  if (!tempBuf) {
    fclose(fsrc);
    return -1;
  }

  size_t bytesRead;
  while ((bytesRead = fread(tempBuf, 1, cluster_size, fsrc)) > 0) {
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
    uint32_t sector_num =
        first_data_sector +
        (currentFileCluster - 2) * image->boot_sector.BPB_SecPerClus;
    uint32_t fileClusterOffset = sector_num * sec_size;

    // Escreve os dados lidos no cluster atual
    fseek(image->file, fileClusterOffset, SEEK_SET);
    fwrite(tempBuf, 1, bytesRead, image->file);
    totalBytesWritten += bytesRead;

    // Se ainda há mais dados para ler, aloca novo cluster
    if (bytesRead == cluster_size) {
      uint32_t newCluster = allocate_free_cluster(image);
      if (newCluster == 0xFFFFFFFF) {
        fprintf(stderr, "Sem espaço para alocar cluster adicional.\n");
        break;
      }

      // Liga o cluster atual ao novo
      image->fat1[currentFileCluster] = newCluster;
      image->fat1[newCluster] = 0x0FFFFFFF; // EOC

      write_fat(image);
      update_fsinfo(image, newCluster);

      currentFileCluster = newCluster;
    }
  }

  // Atualiza o tamanho do arquivo na entrada do diretório
  targetEntry.DIR_FileSize = totalBytesWritten;
  updateDirEntry(dst_path_in_image, &targetEntry, image, current_cluster);

  free(tempBuf);
  fclose(fsrc);

  printf("Copiado arquivo local '%s' para imagem (arquivo '%s', %u bytes).\n",
         src_path, dst_path_in_image, totalBytesWritten);
  return 0;
}

/*
 * Copia um arquivo de dentro da imagem para o sistema local.
 */
static int copyFileImageToHost(const char *src_path_in_image,
                               const char *dst_path, Fat32Image *image,
                               uint32_t current_cluster) {
  // Localiza a entrada do arquivo (src_path_in_image) no diretório atual
  uint32_t fileCluster = 0;
  uint32_t fileSize = 0;
  if (findFileOnCluster(src_path_in_image, &fileCluster, &fileSize, image,
                        current_cluster) < 0) {
    fprintf(stderr, "Arquivo '%s' não encontrado na imagem.\n",
            src_path_in_image);
    return -1;
  }

  // Cria arquivo de destino local
  FILE *fdst = fopen(dst_path, "wb");
  if (!fdst) {
    fprintf(stderr, "Erro ao criar arquivo local '%s': %s\n", dst_path,
            strerror(errno));
    return -1;
  }

  // Lê cluster a cluster a cadeia do arquivo na imagem e escreve em fdst
  uint32_t sec_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = sec_size * image->boot_sector.BPB_SecPerClus;

  uint8_t *tempBuf = malloc(cluster_size);
  if (!tempBuf) {
    fclose(fdst);
    return -1;
  }

  uint32_t currentFileCluster = fileCluster;
  uint32_t bytesRemaining = fileSize;

  while (currentFileCluster != 0xFFFFFFFF && bytesRemaining > 0) {
    // Calcula o setor inicial do cluster que vamos ler
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
    uint32_t sec_num =
        first_data_sector +
        (currentFileCluster - 2) * image->boot_sector.BPB_SecPerClus;

    // Define quantos bytes ainda faltam para ler do arquivo
    uint32_t bytesToRead =
        (bytesRemaining < cluster_size) ? bytesRemaining : cluster_size;

    // Posiciona no arquivo de imagem e lê
    fseek(image->file, sec_num * sec_size, SEEK_SET);
    size_t readBytes = fread(tempBuf, 1, bytesToRead, image->file);

    // Grava no arquivo local
    size_t written = fwrite(tempBuf, 1, readBytes, fdst);
    if (written != readBytes) {
      fprintf(stderr, "Erro ao escrever no arquivo local.\n");
      break;
    }

    bytesRemaining -= readBytes;

    // Se ainda restam bytes, obtemos o próximo cluster da cadeia
    if (bytesRemaining > 0) {
      currentFileCluster = get_next_cluster(image, currentFileCluster);
      if (currentFileCluster == 0xFFFFFFFF) {
        // Fim inesperado da cadeia, arquivo “cortado” (ou corrompido)
        break;
      }
    }
  }

  free(tempBuf);
  fclose(fdst);

  printf("Copiado arquivo da imagem ('%s') para local ('%s').\n",
         src_path_in_image, dst_path);
  return 0;
}

/*
 * Exemplo de lógica principal para "cp":
 *   cp <src> <dst>
 *   - Se <src> começa com "img/", então é arquivo/diretório da imagem;
 *     senão, é do sistema local. O mesmo vale para <dst>.
 */
void cpCommand(char *command, Fat32Image *image, uint32_t current_cluster) {
  // Remove "cp " e espaços
  char *args = command + 2;
  while (*args == ' ')
    args++;

  // Precisamos de src e dst
  char *src = strtok(args, " ");
  char *dst = strtok(NULL, " ");
  if (!src || !dst) {
    fprintf(stderr, "Uso: cp <src> <dst>\n");
    return;
  }

  // Verifica se src está na imagem
  int srcInImage = isImagePath(src);
  // Verifica se dst está na imagem
  int dstInImage = isImagePath(dst);

  // Verifica se é um diretório, caso seja não é permitido copiar
  FAT32_DirEntry srcEntry;
  if (findDirEntry(stripImagePrefix(src), &srcEntry, image, current_cluster) >=
      0) {
    if (srcEntry.DIR_Attr & 0x10) {
      fprintf(stderr, "Não é permitido copiar diretórios.\n");
      return;
    }
  }

  // Se ambos local → local, poderia simplesmente usar a função do sistema
  // (fopen/fwrite). Se local → imagem, chamamos copyFileHostToImage. Se imagem
  // → local, chamamos copyFileImageToHost. Se imagem → imagem, precisamos de
  // uma rotina que leia do src e crie um novo no dst.

  if (!srcInImage && !dstInImage) {
    // Local -> Local (ex.: cp /home/foo.txt /tmp/bar.txt)
    FILE *fsrc = fopen(src, "rb");
    if (!fsrc) {
      fprintf(stderr, "Erro ao abrir '%s': %s\n", src, strerror(errno));
      return;
    }
    FILE *fdst = fopen(dst, "wb");
    if (!fdst) {
      fprintf(stderr, "Erro ao criar '%s': %s\n", dst, strerror(errno));
      fclose(fsrc);
      return;
    }
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fsrc)) > 0) {
      fwrite(buf, 1, n, fdst);
    }
    fclose(fsrc);
    fclose(fdst);
    printf("Copiado local->local: %s -> %s\n", src, dst);
  } else if (!srcInImage && dstInImage) {
    // Local -> Imagem
    const char *dst_path_in_img = stripImagePrefix(dst);
    copyFileHostToImage(src, dst_path_in_img, image, current_cluster);
  } else if (srcInImage && !dstInImage) {
    // Imagem -> Local
    const char *src_path_in_img = stripImagePrefix(src);
    copyFileImageToHost(src_path_in_img, dst, image, current_cluster);
  } else {
    // Imagem -> Imagem
    const char *src_path_in_img = stripImagePrefix(src);
    const char *dst_path_in_img = stripImagePrefix(dst);

    char tmpFile[] = "/tmp/fatshellcpXXXXXX"; // Nome de template
    int fd = mkstemp(tmpFile);
    if (fd < 0) {
      fprintf(stderr, "Falha ao criar arquivo temporário.\n");
      return;
    }
    close(fd);

    // Copia da imagem para tmp
    if (copyFileImageToHost(src_path_in_img, tmpFile, image, current_cluster) <
        0) {
      unlink(tmpFile);
      return;
    }
    // Copia de tmp para imagem (dst)
    copyFileHostToImage(tmpFile, dst_path_in_img, image, current_cluster);

    // remove tmp
    unlink(tmpFile);
    printf("Copiado (imagem->imagem) '%s' -> '%s'.\n", src_path_in_img,
           dst_path_in_img);
  }
}
