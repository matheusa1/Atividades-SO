#include "matriz/matriz.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void writeArchive(int **matrix, int row, int col);

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Uso: %s <nr linhas> <nr colunas>\n", argv[0]);
    return 1;
  }

  int row = atoi(argv[1]);
  int col = atoi(argv[2]);

  int **matrix = create_matrix(row, col);

  generate_elements(matrix, row, col, 100);

  writeArchive(matrix, row, col);

  return 0;
}

void writeArchive(int **matrix, int row, int col) {
  FILE *archive = fopen("matrix.in", "w");

  if (archive == NULL) {
    printf("Erro ao abrir o arquivo\n");
    exit(1);
  }

  fprintf(archive, "%dx%d\n", row, col);

  for (int i = 0; i < row; i++) {
    for (int j = 0; j < col; j++) {
      fprintf(archive, "%d ", matrix[i][j]);
    }
    fprintf(archive, "\n");
  }
}
