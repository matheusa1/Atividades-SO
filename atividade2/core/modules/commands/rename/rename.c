#include "interface.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Função auxiliar para converter uma string (ex: "ARQUIVO.TXT") para o formato
// FAT 8.3 (11 bytes) O resultado é colocado em out, que deve ter espaço para 11
// bytes.
static void string_to_FAT83(const char *input, char out[11]) {
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

// Função que implementa o comando rename.
// Espera: "rename <nome_antigo> <nome_novo>"
void renameCommand(char *command, Fat32Image *image, uint32_t current_cluster) {
  // Pula o comando "rename" (6 caracteres) e os espaços seguintes
  char *args = command + 6;
  while (*args == ' ')
    args++;

  // Separa os dois argumentos
  char *old_name = strtok(args, " ");
  char *new_name = strtok(NULL, " ");

  if (!old_name || !new_name) {
    fprintf(stderr, "Uso: rename <nome_antigo> <nome_novo>\n");
    return;
  }

  char target_name[13];
  {
    char fat_name[11];
    string_to_FAT83(old_name, fat_name);
    convert_to_83(fat_name, target_name);
  }

  // Prepara o novo nome em formato FAT: 11 bytes, sem terminador, com espaços.
  char new_fat_name[11];
  string_to_FAT83(new_name, new_fat_name);

  // Variáveis para percorrer os clusters do diretório
  uint32_t cluster = current_cluster;
  uint32_t sector_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = image->boot_sector.BPB_SecPerClus * sector_size;
  int encontrado = 0;

  do {
    // Calcula o primeiro setor de dados:
    // first_data_sector = setores reservados + (número de FATs * setores por
    // FAT)
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
    // Calcula o setor inicial deste cluster:
    uint32_t sector =
        first_data_sector + (cluster - 2) * image->boot_sector.BPB_SecPerClus;
    // Calcula o offset (em bytes) deste cluster:
    uint32_t cluster_offset = sector * sector_size;

    // Aloca um buffer para o cluster
    uint8_t *buffer = malloc(cluster_size);
    if (!buffer) {
      fprintf(stderr, "Erro de alocação\n");
      return;
    }

    fseek(image->file, cluster_offset, SEEK_SET);
    if (fread(buffer, cluster_size, 1, image->file) != 1) {
      free(buffer);
      fprintf(stderr, "Erro na leitura do cluster\n");
      return;
    }

    // Varre as entradas do cluster
    FAT32_DirEntry *entry = (FAT32_DirEntry *)buffer;
    int entries = cluster_size / sizeof(FAT32_DirEntry);
    for (int i = 0; i < entries; i++) {
      // Se o primeiro byte for 0x00, fim das entradas
      if (entry[i].DIR_Name[0] == 0x00) {
        free(buffer);
        goto fim;
      }
      // Se a entrada estiver apagada ou for volume label, ignora
      if (entry[i].DIR_Name[0] == 0xE5)
        continue;
      if (entry[i].DIR_Attr & 0x08)
        continue;

      // Converte a entrada para string legível (8.3)
      char entry_name[13];
      convert_to_83(entry[i].DIR_Name, entry_name);

      // Compara com o nome antigo buscado
      if (strcmp(entry_name, target_name) == 0) {
        // Calcula a posição exata no arquivo onde está a entrada
        // Cada cluster está no offset cluster_offset; o registro i está a
        // offset (i * sizeof(FAT32_DirEntry)) dentro do cluster.
        long entry_file_offset = cluster_offset + i * sizeof(FAT32_DirEntry);

        // Atualiza os 11 bytes do nome com o novo nome FAT (new_fat_name)
        fseek(image->file, entry_file_offset, SEEK_SET);
        if (fwrite(new_fat_name, sizeof(char), 11, image->file) != 11) {
          fprintf(stderr, "Erro ao escrever o novo nome.\n");
        } else {
          fflush(image->file);
          printf("Arquivo '%s' renomeado para '%s'\n", entry_name, new_name);
        }
        encontrado = 1;
        break;
      }
    }
    free(buffer);
    if (encontrado)
      break;

    // Se não encontrou, busca o próximo cluster do diretório
    uint32_t next_cluster = get_next_cluster(image, cluster);
    if (next_cluster == 0xFFFFFFFF || next_cluster == cluster)
      break;
    cluster = next_cluster;
  } while (1);

fim:
  if (!encontrado) {
    fprintf(stderr, "Arquivo '%s' não encontrado no diretório corrente.\n",
            target_name);
  }
}
