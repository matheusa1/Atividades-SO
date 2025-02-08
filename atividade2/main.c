#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/modules/FAT/interface.h"
#include "core/modules/commands/attr/interface.h"
#include "core/modules/commands/cd/interface.h"
#include "core/modules/commands/ls/interface.h"
#include "core/modules/commands/rename/interface.h"
#include "core/modules/common/interface.h"

// Função para exibir informações básicas da FAT32
void show_info(Fat32Image *image) {
  printf("OEM Name: %s\n", image->boot_sector.BS_OEMName);
  printf("Bytes per sector: %d\n", image->boot_sector.BPB_BytsPerSec);
  printf("Sectors per cluster: %d\n", image->boot_sector.BPB_SecPerClus);
  printf("Reserved sectors: %d\n", image->boot_sector.BPB_RsvdSecCnt);
  printf("Number of FATs: %d\n", image->boot_sector.BPB_NumFATs);
  printf("Sectors per FAT: %d\n", image->boot_sector.BPB_SecPerTrk);
  printf("Root cluster: %d\n", image->boot_sector.BPB_RootClus);
}

// Função para exibir o conteúdo de um cluster no formato de texto
void show_cluster(Fat32Image *image, uint32_t cluster_num) {
  uint32_t fat_size = image->boot_sector.BPB_FATSz32;
  uint32_t reserved_sectors = image->boot_sector.BPB_RsvdSecCnt;
  uint32_t sector_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = compute_cluster_size(image);

  // Offset do início da área de dados
  uint32_t fat_offset = reserved_sectors * sector_size;
  // Tamanho total de cada FAT, em bytes
  uint32_t fat_bytes = fat_size * sector_size;

  // Data offset = inicio da área de dados
  uint32_t data_offset =
      fat_offset + (image->boot_sector.BPB_NumFATs * fat_bytes);
  // Offset em bytes do cluster que vamos ler
  uint32_t cluster_offset = data_offset + (cluster_num - 2) * cluster_size;

  // Move o ponteiro para o início do cluster
  if (fseek(image->file, cluster_offset, SEEK_SET) != 0) {
    printf("Erro: Não foi possível buscar o cluster %d.\n", cluster_num);
    return;
  }

  // Lê o cluster no buffer
  uint8_t *buffer = malloc(cluster_size); // mudamos para cluster_size
  if (!buffer) {
    printf("Erro de alocação de memória.\n");
    return;
  }
  if (fread(buffer, cluster_size, 1, image->file) != 1) {
    printf("Erro: Não foi possível ler o cluster %d.\n", cluster_num);
    free(buffer);
    return;
  }

  // Exibe o conteúdo em formato texto
  for (int i = 0; i < (int)cluster_size; i++) {
    // Exibe caracteres imprimíveis (ASCII 32..126) ou '.' para os
    // não-imprimíveis
    printf("%c", (buffer[i] >= 32 && buffer[i] <= 126) ? buffer[i] : '.');
    if ((i + 1) % 64 == 0)
      printf("\n");
  }
  printf("\n");
  free(buffer);
}

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
  uint32_t *fat2 = (uint32_t *)malloc(fat_size); // Se BPB_NumFATs > 1

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
  while (1) {
    printf("fatshell:[img%s] $ ", currentPath);
    fgets(command, sizeof(command), stdin);

    // Remove o caractere de nova linha do comando
    command[strcspn(command, "\n")] = '\0';

    // Comando "info": exibe informações do disco
    if (strcmp(command, "info") == 0) {
      show_info(&image);
    } else if (strcmp(command, "ls") == 0) {
      list_directory(current_dir, image);
    }
    // Comando "cluster <num>": exibe o conteúdo do cluster especificado
    else if (strncmp(command, "cluster", 7) == 0) {
      uint32_t cluster_num = atoi(&command[8]); // Extrai o número do cluster
      show_cluster(&image, cluster_num);
    } else if (strncmp(command, "cd", 2) == 0) {
      // “cd ” tem 3 caracteres, então o restante é o nome do diretório
      cdCommand(command, &image, currentPath);
    } else if (strncmp(command, "pwd", 3) == 0) {
      printf("%s\n", currentPath);
    } else if (strncmp(command, "attr", 4) == 0) {
      attrCommand(command, &image, current_dir);
    } else if (strncmp(command, "rename", 6) == 0) {
      renameCommand(command, &image, current_dir);
    }
    // Comando "exit": encerra o shell
    else if (strcmp(command, "exit") == 0) {
      break;
    } else {
      printf("Unknown command: %s\n", command);
    }
  }

  // Fecha o arquivo da imagem
  fclose(file);
  return 0;
}
