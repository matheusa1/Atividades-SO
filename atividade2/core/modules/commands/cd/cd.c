#include "interface.h"

void cdCommand(char *command, Fat32Image *image, char *currentPath) {
  char *dirname = &command[3];
  if (strlen(dirname) == 0) {
    fprintf(stderr, "Uso: cd <dirname>\n");
    return;
  }

  dirname = fileToUpper(dirname);

  // Se o usuário digitar "cd /", volta para a raiz
  if (strcmp(dirname, "/") == 0) {
    current_dir = image->boot_sector.BPB_RootClus;
    strcpy(currentPath, "/");
    stack_top = -1; // limpa o histórico
    return;
  }

  // Se for "cd ..", volta para o diretório pai (se possível)
  if (strcmp(dirname, "..") == 0) {
    if (stack_top >= 0) {
      current_dir = dir_stack[stack_top]; // recupera o diretório anterior
      stack_top--;                        // remove da pilha

      // Atualiza currentPath: remove o último segmento após a última '/'.
      char *lastSlash = strrchr(currentPath, '/');
      if (lastSlash != NULL && lastSlash != currentPath) {
        *lastSlash = '\0';
      } else {
        strcpy(currentPath, "/");
      }
    } else {
      // Se a pilha estiver vazia, já estamos na raiz.
      current_dir = image->boot_sector.BPB_RootClus;
      strcpy(currentPath, "/");
    }
    return;
  }

  // Agora procuramos o cluster do subdiretório
  uint32_t newCluster = find_directory_cluster(image, current_dir, dirname);
  if (newCluster == 0xFFFFFFFF) {
    fprintf(stderr, "Diretório '%s' não encontrado.\n", dirname);
  } else {
    if (stack_top < MAX_PATH_DEPTH - 1) {
      dir_stack[++stack_top] = current_dir;
    } else {
      fprintf(stderr, "Limite de histórico de diretórios atingido.\n");
    }
    current_dir = newCluster;

    // Atualiza o caminho para exibição
    if (strcmp(currentPath, "/") == 0) {
      snprintf(currentPath, sizeof(currentPath), "/%s", dirname);
    } else {
      strncat(currentPath, "/", sizeof(currentPath) - strlen(currentPath) - 1);
      strncat(currentPath, dirname,
              sizeof(currentPath) - strlen(currentPath) - 1);
    }
  }
}
