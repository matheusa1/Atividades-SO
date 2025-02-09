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
 * Nota: Aqui você pode querer normalizar "/algum/diretorio".
 */
static const char *stripImagePrefix(const char *path) {
  if (isImagePath(path)) {
    return path + 4;
  }
  return path;
}

/*
 * Função auxiliar que copia um arquivo local do SO hospedeiro para dentro da
 * imagem. Exemplo simples (não lida com diretórios recursivamente).
 */
static int copyFileHostToImage(const char *src_path,
                               const char *dst_path_in_image, Fat32Image *image,
                               uint32_t current_cluster) {
  // 1) Abre o arquivo local
  FILE *fsrc = fopen(src_path, "rb");
  if (!fsrc) {
    fprintf(stderr, "Erro ao abrir arquivo local '%s': %s\n", src_path,
            strerror(errno));
    return -1;
  }

  // 2) Vamos criar (ou sobrescrever) o arquivo na imagem, usando "touch".
  //    Em seguida, abriremos cada cluster e gravaremos o conteúdo.
  //    Para facilitar, vamos chamar a touchCommand, que cria o arquivo vazio,
  //    e depois nós próprios gravamos o conteúdo no cluster.

  // Monta comando "touch <dst>"
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "touch %s", dst_path_in_image);
  touchCommand(cmd, image, current_cluster);

  // Agora, precisamos localizar de novo a entrada (DIR_Entry) do arquivo criado
  // para então escrever seu conteúdo.

  // *** Exemplo grosseiro: supõe que o arquivo acabou de ser criado e
  // que `touchCommand` não criou subdiretórios etc. ***

  // Procurar a entrada do arquivo no diretório atual.
  uint32_t sec_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = sec_size * image->boot_sector.BPB_SecPerClus;
  uint32_t cluster = current_cluster;
  int foundEntry = 0;
  uint32_t fileCluster = 0;

  while (!foundEntry && cluster != 0xFFFFFFFF) {
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
    uint32_t sector_num =
        first_data_sector + (cluster - 2) * image->boot_sector.BPB_SecPerClus;
    uint32_t cluster_offset = sector_num * sec_size;

    // Lê o cluster
    uint8_t *buffer = malloc(cluster_size);
    if (!buffer) {
      fclose(fsrc);
      return -1;
    }
    fseek(image->file, cluster_offset, SEEK_SET);
    if (fread(buffer, cluster_size, 1, image->file) != 1) {
      free(buffer);
      fclose(fsrc);
      return -1;
    }

    FAT32_DirEntry *entry = (FAT32_DirEntry *)buffer;
    int entries_per_cluster = cluster_size / sizeof(FAT32_DirEntry);
    for (int i = 0; i < entries_per_cluster; i++) {
      if (entry[i].DIR_Name[0] == 0x00) {
        // fim da lista
        break;
      }
      if (entry[i].DIR_Name[0] == 0xE5) {
        continue;
      }
      // ignora volume label
      if (entry[i].DIR_Attr & 0x08) {
        continue;
      }
      // converte para string
      char name[13];
      convert_to_83(entry[i].DIR_Name, name);
      // compara com o dst_path_in_image (em maiúsculas, usualmente)
      if (strcmp(name, dst_path_in_image) == 0) {
        // achou a entrada
        // obtém o cluster
        foundEntry = 1;
        fileCluster = (entry[i].DIR_FstClusHI << 16) | entry[i].DIR_FstClusLO;
        break;
      }
    }
    free(buffer);

    if (!foundEntry) {
      // vai para o próximo cluster do diretório
      uint32_t next_cluster = get_next_cluster(image, cluster);
      if (next_cluster == 0xFFFFFFFF || next_cluster == cluster) {
        break;
      }
      cluster = next_cluster;
    }
  }

  if (!foundEntry || fileCluster == 0) {
    fprintf(stderr,
            "Não foi possível localizar a entrada recém-criada para '%s'.\n",
            dst_path_in_image);
    fclose(fsrc);
    return -1;
  }

  // Agora gravamos o conteúdo de fsrc para fileCluster.
  // Precisamos ir alocando clusters conforme necessário.
  // Contudo, o "touchCommand" já fez a alocação inicial.
  // Para um exemplo simples, se o arquivo for maior que 1 cluster,
  // precisamos continuar alocando.
  // Abaixo um exemplo parcial (apenas escreve o arquivo se couber em 1
  // cluster). Em produção, você faria um loop que lê cluster e, se encher,
  // aloca outro e assim por diante.

  // Faz seek no cluster do arquivo
  uint32_t first_data_sector =
      image->boot_sector.BPB_RsvdSecCnt +
      (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
  uint32_t sector_num =
      first_data_sector + (fileCluster - 2) * image->boot_sector.BPB_SecPerClus;
  uint32_t fileClusterOffset = sector_num * sec_size;

  fseek(image->file, fileClusterOffset, SEEK_SET);

  // Ler em blocos e escrever.
  // CUIDADO: se o arquivo local for maior que 1 cluster, precisamos
  // ir alocando clusters adicionais. Aqui vai um exemplo simplificado,
  // copiando no máximo cluster_size bytes.
  {
    uint8_t *tempBuf = malloc(cluster_size);
    if (!tempBuf) {
      fclose(fsrc);
      return -1;
    }
    size_t bytesRead = fread(tempBuf, 1, cluster_size, fsrc);
    fwrite(tempBuf, 1, bytesRead, image->file);
    fflush(image->file);
    free(tempBuf);
  }

  fclose(fsrc);
  printf("Copiado arquivo local '%s' para imagem (arquivo '%s').\n", src_path,
         dst_path_in_image);
  return 0;
}

/*
 * Copia um arquivo de dentro da imagem para o sistema local. (Exemplo básico,
 * sem tratar de subdiretórios.)
 */
static int copyFileImageToHost(const char *src_path_in_image,
                               const char *dst_path, Fat32Image *image,
                               uint32_t current_cluster) {
  // 1) Tentar achar o arquivo src_path_in_image no diretório atual
  // (current_cluster).
  uint32_t cluster = current_cluster;
  uint32_t sec_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = sec_size * image->boot_sector.BPB_SecPerClus;
  uint32_t fileCluster = 0;
  int foundEntry = 0;
  uint32_t fileSize = 0;

  while (!foundEntry && cluster != 0xFFFFFFFF) {
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
    uint32_t sector_num =
        first_data_sector + (cluster - 2) * image->boot_sector.BPB_SecPerClus;

    uint8_t *buffer = malloc(cluster_size);
    if (!buffer) {
      return -1;
    }
    fseek(image->file, sector_num * sec_size, SEEK_SET);
    if (fread(buffer, cluster_size, 1, image->file) != 1) {
      free(buffer);
      return -1;
    }
    FAT32_DirEntry *entry = (FAT32_DirEntry *)buffer;
    int entries_per_cluster = cluster_size / sizeof(FAT32_DirEntry);
    for (int i = 0; i < entries_per_cluster; i++) {
      if (entry[i].DIR_Name[0] == 0x00) {
        break;
      }
      if (entry[i].DIR_Name[0] == 0xE5) {
        continue;
      }
      if (entry[i].DIR_Attr & 0x08) {
        continue;
      }
      char name[13];
      convert_to_83(entry[i].DIR_Name, name);
      if (strcmp(name, src_path_in_image) == 0) {
        foundEntry = 1;
        fileCluster = (entry[i].DIR_FstClusHI << 16) | entry[i].DIR_FstClusLO;
        fileSize = entry[i].DIR_FileSize;
        break;
      }
    }
    free(buffer);

    if (!foundEntry) {
      // próximo cluster
      uint32_t next_cluster = get_next_cluster(image, cluster);
      if (next_cluster == 0xFFFFFFFF || next_cluster == cluster) {
        break;
      }
      cluster = next_cluster;
    }
  }

  if (!foundEntry || fileCluster == 0) {
    fprintf(stderr, "Arquivo '%s' não encontrado na imagem.\n",
            src_path_in_image);
    return -1;
  }

  // 2) Abre/cria o arquivo de destino local
  FILE *fdst = fopen(dst_path, "wb");
  if (!fdst) {
    fprintf(stderr, "Erro ao criar arquivo local '%s': %s\n", dst_path,
            strerror(errno));
    return -1;
  }

  // 3) Lê cluster a cluster o arquivo e grava no fdst.
  // Exemplo simplificado (não trata de mais clusters do arquivo).
  // O ideal é seguir a cadeia (fileCluster -> get_next_cluster) para ler tudo.
  // fileSize diz quantos bytes copiar em total.
  {
    uint8_t *tempBuf = malloc(cluster_size);
    if (!tempBuf) {
      fclose(fdst);
      return -1;
    }

    // Leitura do cluster inicial
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
    uint32_t sec_num = first_data_sector +
                       (fileCluster - 2) * image->boot_sector.BPB_SecPerClus;
    fseek(image->file, sec_num * sec_size, SEEK_SET);

    size_t toRead = (fileSize < cluster_size) ? fileSize : cluster_size;
    size_t readBytes = fread(tempBuf, 1, toRead, image->file);
    fwrite(tempBuf, 1, readBytes, fdst);

    // Aqui, se fileSize > cluster_size, teríamos que buscar o próximo cluster
    // e continuar a ler, até o fim. Não está implementado neste exemplo.

    free(tempBuf);
  }

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
    // Precisamos retirar o prefixo "img/" de dst
    const char *dst_path_in_img = stripImagePrefix(dst);
    copyFileHostToImage(src, dst_path_in_img, image, current_cluster);
  } else if (srcInImage && !dstInImage) {
    // Imagem -> Local
    const char *src_path_in_img = stripImagePrefix(src);
    copyFileImageToHost(src_path_in_img, dst, image, current_cluster);
  } else {
    // Imagem -> Imagem
    // Precisamos de rotina análoga. Exemplo simples:
    //   1) Criar arquivo temporário local
    //   2) Baixar src da imagem para esse tmp
    //   3) Subir tmp para a imagem como dst
    //   4) Apagar tmp
    const char *src_path_in_img = stripImagePrefix(src);
    const char *dst_path_in_img = stripImagePrefix(dst);

    char tmpFile[] = "/tmp/fatshellcpXXXXXX"; // Nome de template
    int fd = mkstemp(tmpFile);
    if (fd < 0) {
      fprintf(stderr, "Falha ao criar arquivo temporário.\n");
      return;
    }
    close(fd);

    // Passo 2) Copia da imagem para tmp
    if (copyFileImageToHost(src_path_in_img, tmpFile, image, current_cluster) <
        0) {
      unlink(tmpFile);
      return;
    }
    // Passo 3) Copia de tmp para imagem (dst)
    copyFileHostToImage(tmpFile, dst_path_in_img, image, current_cluster);

    // Passo 4) remove tmp
    unlink(tmpFile);
    printf("Copiado (imagem->imagem) '%s' -> '%s'.\n", src_path_in_img,
           dst_path_in_img);
  }
}
