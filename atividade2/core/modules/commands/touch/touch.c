#include "interface.h"
#include <stdlib.h>

// Cria um arquivo vazio
void touchCommand(char *command, Fat32Image *image, uint32_t current_cluster) {
  char *filename = &command[6];
  while (*filename == ' ') {
    filename++;
  }
  if (strlen(filename) == 0) {
    printf("Usage: touch <filename>\n");
    return;
  }

  filename = fileToUpper(filename);

  // Converte o nome do arquivo para FAT 8.3
  char fat_filename[12];
  string_to_FAT83(filename, fat_filename);

  // Lendo os clusters do diretório atual
  uint32_t sec_size = image->boot_sector.BPB_BytsPerSec;
  uint32_t cluster_size = sec_size * image->boot_sector.BPB_SecPerClus;

  // Preparando a data e hora atuais
  time_t rawtime = time(NULL);
  struct tm *lt = localtime(&rawtime);

  uint16_t fat_date = dateToFatDate(lt);
  uint16_t fat_time = timeToFatTime(lt);

  uint32_t dir_cluster = current_cluster;

  int encontrou = 0;
  while (1) {
    // Calcular início do cluster do diretório
    uint32_t first_data_sector =
        image->boot_sector.BPB_RsvdSecCnt +
        (image->boot_sector.BPB_NumFATs * image->boot_sector.BPB_FATSz32);
    uint32_t sector_num = first_data_sector +
                          (dir_cluster - 2) * image->boot_sector.BPB_SecPerClus;
    uint32_t cluster_offset = sector_num * sec_size;

    // Lê o cluster
    uint8_t *buffer = (uint8_t *)malloc(cluster_size);
    if (!buffer) {
      fprintf(stderr, "Falta de memória.\n");
      return;
    }
    fseek(image->file, cluster_offset, SEEK_SET);
    if (fread(buffer, cluster_size, 1, image->file) != 1) {
      free(buffer);
      fprintf(stderr, "Erro ao ler cluster.\n");
      return;
    }

    // Percorre as entradas de diretório
    FAT32_DirEntry *entry = (FAT32_DirEntry *)buffer;
    int entries_per_cluster = cluster_size / sizeof(FAT32_DirEntry);
    int encontrou_espaco_vazio = -1; // índice onde podemos criar
    for (int i = 0; i < entries_per_cluster; i++) {
      // 0x00 => fim das entradas
      if (entry[i].DIR_Name[0] == 0x00) {
        // Achamos um slot livre "implícito" (fim), podemos usar.
        encontrou_espaco_vazio = i;
        break;
      }
      // 0xE5 => entrada livre, podemos reaproveitar
      if ((unsigned char)entry[i].DIR_Name[0] == 0xE5 &&
          encontrou_espaco_vazio < 0) {
        encontrou_espaco_vazio = i;
        // continuamos, pois podemos achar se o arquivo já existe
      }
      // Se a entrada não está livre, checar se é o arquivo que buscamos
      if ((unsigned char)entry[i].DIR_Name[0] != 0xE5 &&
          !(entry[i].DIR_Attr & 0x08)) {
        // converte para string
        char existingName[13];
        convert_to_83(entry[i].DIR_Name, existingName);
        // compare com arg (o nome digitado), mas note que “existingName”
        // foi convertido de “entry[i].DIR_Name”
        if (strcmp(existingName, filename) == 0) {
          // Arquivo já existe
          // => Atualiza datas e, se não tiver cluster, alocar
          encontrou = 1;
          uint32_t old_cluster =
              ((uint32_t)entry[i].DIR_FstClusHI << 16) | entry[i].DIR_FstClusLO;

          // Atualiza timestamps
          entry[i].DIR_WrtDate = fat_date;
          entry[i].DIR_WrtTime = fat_time;
          entry[i].DIR_LstAccDate = fat_date;

          // Se não tiver cluster ainda, alocar e zerar
          if (old_cluster == 0) {
            uint32_t newc = allocate_free_cluster(image);
            if (newc == 0xFFFFFFFF) {
              fprintf(stderr, "Sem espaço para alocar cluster de dados.\n");
              free(buffer);
              return;
            }
            // Atualiza FAT no disco
            write_fat(image);
            // Atualiza FSInfo no disco
            update_fsinfo(image, newc);

            // Grava no entry
            entry[i].DIR_FstClusHI = (uint16_t)((newc >> 16) & 0xFFFF);
            entry[i].DIR_FstClusLO = (uint16_t)(newc & 0xFFFF);

            // Zera esse cluster de dados
            uint8_t *zero_buf = calloc(1, cluster_size);
            uint32_t new_sec_num =
                first_data_sector +
                (newc - 2) * image->boot_sector.BPB_SecPerClus;
            fseek(image->file, new_sec_num * sec_size, SEEK_SET);
            fwrite(zero_buf, cluster_size, 1, image->file);
            fflush(image->file);
            free(zero_buf);
          }

          // Grava de volta a entrada atualizada
          fseek(image->file, cluster_offset + i * sizeof(FAT32_DirEntry),
                SEEK_SET);
          fwrite(&entry[i], sizeof(FAT32_DirEntry), 1, image->file);
          fflush(image->file);

          printf("Arquivo '%s' já existia; datas atualizadas.\n", filename);

          free(buffer);
          return;
        }
      }
    }

    // Se já achamos um espaço ali (fim ou E5), podemos criar agora
    // (se não encontramos o arquivo)
    if (!encontrou && encontrou_espaco_vazio >= 0) {
      // Prepara a entrada
      FAT32_DirEntry newEntry;
      memset(&newEntry, 0, sizeof(FAT32_DirEntry));
      memcpy(newEntry.DIR_Name, fat_filename, 11);
      newEntry.DIR_Attr = 0x20; // Arquivo
      newEntry.DIR_FileSize = 0;
      newEntry.DIR_CrtDate = fat_date;
      newEntry.DIR_CrtTime = fat_time;
      newEntry.DIR_WrtDate = fat_date;
      newEntry.DIR_WrtTime = fat_time;
      newEntry.DIR_LstAccDate = fat_date;

      // Aloca cluster para o conteúdo do arquivo
      uint32_t file_cluster = allocate_free_cluster(image);
      if (file_cluster == 0xFFFFFFFF) {
        fprintf(stderr, "Sem cluster livre para o arquivo.\n");
        free(buffer);
        return;
      }
      // flush e update FSInfo
      write_fat(image);
      update_fsinfo(image, file_cluster);

      // Preenche hi/lo
      newEntry.DIR_FstClusHI = (uint16_t)((file_cluster >> 16) & 0xFFFF);
      newEntry.DIR_FstClusLO = (uint16_t)(file_cluster & 0xFFFF);

      // Zera o cluster
      {
        uint8_t *zero_buf = calloc(1, cluster_size);
        uint32_t new_sec_num =
            first_data_sector +
            (file_cluster - 2) * image->boot_sector.BPB_SecPerClus;
        fseek(image->file, new_sec_num * sec_size, SEEK_SET);
        fwrite(zero_buf, cluster_size, 1, image->file);
        fflush(image->file);
        free(zero_buf);
      }

      // Grava no diretório
      fseek(image->file,
            cluster_offset + encontrou_espaco_vazio * sizeof(FAT32_DirEntry),
            SEEK_SET);
      fwrite(&newEntry, sizeof(FAT32_DirEntry), 1, image->file);
      fflush(image->file);

      printf("Arquivo '%s' criado com cluster de dados alocado.\n", filename);
      free(buffer);
      return;
    }

    // Se acabou de varrer esse cluster e não achou o arquivo nem espaço,
    // precisamos ir para o próximo cluster do diretório
    uint32_t next_cluster = get_next_cluster(image, dir_cluster);
    if (next_cluster == 0xFFFFFFFF || next_cluster == dir_cluster) {
      // Fim da cadeia
      // Aqui vamos ALLOCAR um novo cluster para o diretório, linká-lo
      // ao 'dir_cluster' e CRIAR ENTRADA no começo dele.
      uint32_t new_dir_cluster = allocate_free_cluster(image);
      if (new_dir_cluster == 0xFFFFFFFF) {
        fprintf(stderr,
                "Não há espaço para alocar novo cluster de diretório.\n");
        free(buffer);
        return;
      }
      // Marca next_cluster do dir_cluster = new_dir_cluster
      image->fat1[dir_cluster] = (image->fat1[dir_cluster] & 0xF0000000) |
                                 (new_dir_cluster & 0x0FFFFFFF);
      // E o new_dir_cluster aponta para EOC
      image->fat1[new_dir_cluster] = 0x0FFFFFFF;

      write_fat(image);
      update_fsinfo(image, new_dir_cluster);

      // zera o novo cluster
      uint8_t *zero_buf = calloc(1, cluster_size);
      uint32_t new_dir_sector =
          first_data_sector +
          (new_dir_cluster - 2) * image->boot_sector.BPB_SecPerClus;
      fseek(image->file, new_dir_sector * sec_size, SEEK_SET);
      fwrite(zero_buf, cluster_size, 1, image->file);
      fflush(image->file);
      free(zero_buf);

      // Agora criamos a entrada no começo do novo cluster:
      FAT32_DirEntry newEntry;
      memset(&newEntry, 0, sizeof(FAT32_DirEntry));
      memcpy(newEntry.DIR_Name, fat_filename, 11);
      newEntry.DIR_Attr = 0x20; // arquivo
      newEntry.DIR_FileSize = 0;
      newEntry.DIR_CrtDate = fat_date;
      newEntry.DIR_CrtTime = fat_time;
      newEntry.DIR_WrtDate = fat_date;
      newEntry.DIR_WrtTime = fat_time;
      newEntry.DIR_LstAccDate = fat_date;

      // Aloca cluster para conteúdo do arquivo
      uint32_t file_cluster = allocate_free_cluster(image);
      if (file_cluster == 0xFFFFFFFF) {
        fprintf(stderr, "Sem espaço para cluster de arquivo.\n");
        free(buffer);
        return;
      }
      write_fat(image);
      update_fsinfo(image, file_cluster);

      newEntry.DIR_FstClusHI = (uint16_t)((file_cluster >> 16) & 0xFFFF);
      newEntry.DIR_FstClusLO = (uint16_t)(file_cluster & 0xFFFF);

      // agora zera o cluster do arquivo
      uint8_t *file_buf = calloc(1, cluster_size);
      uint32_t file_sec_num =
          first_data_sector +
          (file_cluster - 2) * image->boot_sector.BPB_SecPerClus;
      fseek(image->file, file_sec_num * sec_size, SEEK_SET);
      fwrite(file_buf, cluster_size, 1, image->file);
      fflush(image->file);
      free(file_buf);

      // escreve a entrada no início do novo cluster de diretório
      fseek(image->file, new_dir_sector * sec_size, SEEK_SET);
      fwrite(&newEntry, sizeof(FAT32_DirEntry), 1, image->file);
      fflush(image->file);

      printf("Novo cluster de diretório alocado; arquivo '%s' criado.\n",
             filename);

      free(buffer);
      return;
    }

    free(buffer);
    dir_cluster = next_cluster;
  }
}
