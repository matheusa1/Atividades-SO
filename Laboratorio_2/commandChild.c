// Descrição: Programa que executa um comando passado como argumento.
// Autor: Matheus Andrade
// Data: 2024-10-24

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uso: %s <comando> [argumentos...]\n", argv[0]);
        return 1;
    }

    char *command = argv[1];
    char *args[argc];

    args[0] = command;
    for (int i = 1; i < argc - 1; i++) {
        args[i] = argv[i + 1];
    }

    args[argc - 1] = NULL;

    pid_t pid = fork();
    if(pid) {
        wait(NULL);
        printf("\n\nProcesso pai finalizado.\n\n");
        return 0;
    }

    execvp(command, args);

    return 0;
}
