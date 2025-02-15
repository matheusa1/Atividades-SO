#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/modules/FAT/interface.h"
#include "core/modules/commands/attr/interface.h"
#include "core/modules/commands/cd/interface.h"
#include "core/modules/commands/cp/interface.h"
#include "core/modules/commands/fat_cluster/interface.h"
#include "core/modules/commands/info/interface.h"
#include "core/modules/commands/ls/interface.h"
#include "core/modules/commands/mkdir/interface.h"
#include "core/modules/commands/mv/interface.h"
#include "core/modules/commands/rename/interface.h"
#include "core/modules/commands/rm/interface.h"
#include "core/modules/commands/rmdir/interface.h"
#include "core/modules/commands/touch/interface.h"
#include "core/modules/common/interface.h"

int main(int argc, char *argv[]) {
  // Verifica se o nome da imagem foi fornecido como argumento
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <image_file>\n", argv[0]);
    return 1;
  }

  // Abre o arquivo da imagem no modo leitura binária
  FILE *file = fopen(argv[1], "rb+");
  if (!file) {
    perror("Failed to open image file");
    return 1;
  }

  // Inicializa a estrutura da imagem FAT32
  Fat32Image image;
  image.file = file;

  FAT32_BootSector boot_sector;

  // Carrega o setor de inicialização do FAT32
  if (fread(&boot_sector, sizeof(boot_sector), 1, file) != 1) {
    perror("Fail to read boot sector");
    fclose(file);
    return 1;
  }
  image.boot_sector = boot_sector;

  if (image.boot_sector.BPB_BytsPerSec == 0 ||
      image.boot_sector.BPB_SecPerClus == 0 ||
      image.boot_sector.BPB_RsvdSecCnt == 0 ||
      image.boot_sector.BPB_FATSz32 == 0) {
    fprintf(stderr, "Valores inválidos no boot sector.\n");
    fclose(file);
    return 1;
  }

  // Carrega a estrutura FSInfo
  FSInfoStruct fs_info;
  fseek(file, boot_sector.BPB_FSInfo * boot_sector.BPB_BytsPerSec, SEEK_SET);
  fread(&fs_info, sizeof(fs_info), 1, file);
  image.fs_info = fs_info;

  // Tamanho do FAT em bytes
  uint32_t fat_size =
      image.boot_sector.BPB_FATSz32 * image.boot_sector.BPB_BytsPerSec;

  // Alocando memória para cada FAT
  uint32_t *fat1 = (uint32_t *)malloc(fat_size);
  if (!fat1) {
    perror("Falha to allocate FAT1");
    fclose(file);
    return 1;
  }

  // Carregar FAT1
  fseek(file,
        image.boot_sector.BPB_RsvdSecCnt * image.boot_sector.BPB_BytsPerSec,
        SEEK_SET);
  if (fread(fat1, 1, fat_size, file) != fat_size) {
    perror("Error to read FAT1");
    free(fat1);
    fclose(file);
    return 1;
  }

  image.fat1 = fat1;

  // diretório inicial
  current_dir = image.boot_sector.BPB_RootClus;
  char currentPath[256] = "/";

  // Loop para o shell interativo
  char command[256];
  // char* pwd = pwd_command(directory);
  while (1) {
    printf("fatshell:[img%s] $ ", currentPath);
    fgets(command, sizeof(command), stdin);

    // Remove o caractere de nova linha do comando
    command[strcspn(command, "\n")] = '\0';

    // Comando "info": exibe informações do disco
    if (strcmp(command, "info") == 0) {
      Fat_show_info(&image);
    } else if (strcmp(command, "ls") == 0) {
      list_directory(current_dir, image);
    } else if (strncmp(command, "cluster", 7) == 0) {
      uint32_t cluster_num = atoi(&command[8]); // Extrai o número do cluster
      show_cluster(&image, cluster_num);
    } else if (strncmp(command, "cd", 2) == 0) {
      cdCommand(command, &image, currentPath);
    } else if (strncmp(command, "pwd", 3) == 0) {
      printf("%s\n", currentPath);
    } else if (strncmp(command, "rmdir", 5) == 0) {
      rmdirCommand(command, &image, current_dir);
    } else if (strncmp(command, "attr", 4) == 0) {
      attrCommand(command, &image, current_dir);
    } else if (strncmp(command, "rename", 6) == 0) {
      renameCommand(command, &image, current_dir);
    } else if (strncmp(command, "touch", 5) == 0) {
      touchCommand(command, &image, current_dir);
    } else if (strncmp(command, "mkdir", 5) == 0) {
      mkdirCommand(command, &image, current_dir);
    } else if (strncmp(command, "rm", 2) == 0) {
      rm_command(command, &image, current_dir);
    } else if (strncmp(command, "cp", 2) == 0) {
      cpCommand(command, &image, current_dir);
    } else if (strncmp(command, "mv", 2) == 0) {
      mvCommand(command, &image, current_dir);
    } else if (strcmp(command, "exit") == 0) {
      // Comando "exit": encerra o shell
      break;
    } else {
      printf("Unknown command: %s\n", command);
    }
  }

  // Fecha o arquivo da imagem
  fclose(file);
  return 0;
}
