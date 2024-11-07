// Descrição: Programa que divide um array em n partes e cria n processos filhos para procurar um valor em cada parte
// Autor: Matheus Andrade
// Data: 2024-10-24

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

int searchArray(int* array, int start, int end, int value);

int main() {
    int arraySize;
    int* array;
    int numOfForks;
    int searchNumber;

    printf("\nDigite a quantidade de filhos\n > ");
    scanf("%d", &numOfForks);

    printf("\nQual o tamanho do vetor?\n > ");
    scanf("%d", &arraySize);

    array = (int*) malloc(arraySize * sizeof(int));

    printf("Array: [");
    for(int i = 0; i < arraySize; i++) {
        array[i] = rand() % arraySize;
        printf("%d, ", array[i]);
    }
    printf("\b\b]\n");

    
    printf("\nQual o valor a ser procurado?\n > ");
    scanf("%d", &searchNumber);

    int sizePartPerSon;
    if(arraySize > numOfForks) {
        sizePartPerSon = arraySize / numOfForks;
    } else {
        sizePartPerSon = 1;
        numOfForks = arraySize;
    }

    pid_t pid = 1;

    for(int i = 0; i < numOfForks; i++) {
        if(!pid) break;
        pid = fork();
        if(!pid) {
            int start = i * sizePartPerSon;
            int end = (i == numOfForks - 1) ? arraySize - 1 : (start + sizePartPerSon - 1);
            int result = searchArray(array, start, end, searchNumber);
            if(result == -1) {
                printf("\nNão encontrado na faixa %d até %d pelo processo filho de pid: %d\n",start, end, getpid());
                return 0;
                };
            printf("\nEstá na posição %d No processo filho de pid %d que buscou na faixa %d até %d\n", result, getpid(), start, end);
        return 0;
        }
    }

}

int searchArray(int* array, int start, int end, int value) {
    for(int i = start; i <= end; i++) {
        if(array[i] == value) return i;
    }
    return -1;
}