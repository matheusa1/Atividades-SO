// Autor: Matheus Santos de Andrade, João Vitor Girotto , Victórya Carolina
// Guimarães da Luz
// Data: 2024-10-28

#include "matriz/matriz.h"
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Implemente um programa multithread com pthreads que calcule:
// a) a média aritmética de cada linha de uma matriz MxN e devolva o resultado
// em um vetor de tamanho M.
//
// b) a média geométrica de cada coluna de uma matriz
// MxN e devolva o resultado em um vetor de tamanho N.
//
// O programa deve gerar matrizes MxN com elementos aleatórios para arquivos;
// usar técnicas de paralelização de funções e de dados; ler matrizes MxN de
// arquivos no formato em anexo; gravar os resultados em um arquivo texto

typedef struct AverageArgs {
  int start;
  int end;
  int rowSize;
  int colSize;
  int **matrix;
} AverageArgs;

// Calcula a média aritmética de cada linha da matriz
void *averageRow(void *args);

// Cria a struct de dados para a thread
AverageArgs *createStruct();

// Calcula a média geométrica de cada coluna da matriz
void *averageCol(void *args);

// Arquivo de resultado
FILE *resultFile;

// Abre o arquivo de resultado
void openResultFile() { resultFile = fopen("result.txt", "w"); }

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Uso: %s <numero de threads> <nome_arquivo_entrada>\n", argv[0]);
    return 1;
  }

  int threadCount = atoi(argv[1]);
  char *archiveName = argv[2];

  int row, col;

  openResultFile();
  int **matrix = read_matrix_from_file(archiveName, &row, &col);

  pthread_t threads[threadCount];

  int sizePartPerThread;

  if (row >= threadCount) {
    sizePartPerThread = floorf((float)row / threadCount);
  } else {
    sizePartPerThread = 1;
    threadCount = row;
    printf("Menos threads que linhas, a quantidade de threads foi reajustada "
           "para 1 por linha \n");
  }

  for (int i = 0; i < threadCount; i++) {
    AverageArgs *args = createStruct();

    int start = i * sizePartPerThread;
    int end =
        (i == threadCount - 1) ? row - 1 : (start + sizePartPerThread - 1);

    args->start = start;
    args->end = end;
    args->rowSize = row;
    args->colSize = col;
    args->matrix = matrix;

    pthread_create(&threads[i], NULL, averageRow, (void *)args);
  }

  if (col >= threadCount) {
    sizePartPerThread = floorf((float)col / threadCount);
  } else {
    sizePartPerThread = 1;
    threadCount = col - 1;
    printf("Menos threads que colunas, a quantidade de threads foi reajustada "
           "para 1 por coluna \n");
  }

  for (int i = 0; i < threadCount; i++) {
    AverageArgs *args = createStruct();

    int start = i * sizePartPerThread;
    int end =
        (i == threadCount - 1) ? col - 1 : (start + sizePartPerThread - 1);

    args->start = start;
    args->end = end;
    args->rowSize = row;
    args->colSize = col;
    args->matrix = matrix;

    pthread_create(&threads[i], NULL, averageCol, (void *)args);
  }

  printf("Resultado salvo em result.txt\n");

  pthread_exit(NULL);
}

AverageArgs *createStruct() {
  return (AverageArgs *)malloc(sizeof(AverageArgs));
}

void *averageRow(void *args) {
  AverageArgs *averageArgs = (AverageArgs *)args;

  int start = averageArgs->start;
  int end = averageArgs->end;
  int **matrix = averageArgs->matrix;
  int rowSize = averageArgs->rowSize;
  int colSize = averageArgs->colSize;

  float result[rowSize];

  for (int i = start; i <= end; i++) {
    unsigned int sum = 0;
    for (int j = 0; j < colSize; j++) {
      sum += matrix[i][j];
    }
    result[i - start] = (float)sum / colSize;
  }

  for (int i = start; i <= end; i++) {
    fprintf(resultFile, "Média Linha [%d...%d][%d] = %f\n", start, end, i,
            result[i - start]);
  }

  pthread_exit(NULL);
}

void *averageCol(void *args) {
  AverageArgs *averageArgs = (AverageArgs *)args;

  int start = averageArgs->start;
  int end = averageArgs->end;
  int **matrix = averageArgs->matrix;
  int rowSize = averageArgs->rowSize;
  int colSize = averageArgs->colSize;

  float result[colSize];

  for (int i = start; i <= end; i++) {
    float multiply = 1;
    for (int j = 0; j < rowSize; j++) {
      multiply *= matrix[j][i];
    }
    result[i - start] = pow(multiply, 1.0 / rowSize);
  }

  for (int i = start; i <= end; i++) {
    fprintf(resultFile, "Média Coluna [%d...%d][%d] = %f\n", start, end, i,
            result[i - start]);
  }

  pthread_exit(NULL);
}
